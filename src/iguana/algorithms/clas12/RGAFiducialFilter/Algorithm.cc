#include "Algorithm.h"
#include "iguana/algorithms/TypeDefs.h"
#include "iguana/services/YAMLReader.h"

#include <algorithm>   // clamp, min, max
#include <array>
#include <atomic>
#include <cmath>       // sqrt
#include <cstdlib>     // getenv
#include <functional>  // hash
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace {
std::atomic<int> g_dbg_events_seen{0};   // TU-local to avoid header changes
}

namespace iguana::clas12 {

// --- helpers ---------------------------------------------------------------

static bool banklist_has(hipo::banklist& banks, const char* name) {
  for (auto& b : banks) if (b.getSchema().getName() == name) return true;
  return false;
}

static std::vector<std::pair<float,float>>
to_windows_flat(const std::vector<double>& v) {
  std::vector<std::pair<float,float>> w;
  w.reserve(v.size() / 2);
  for (size_t i = 0; i + 1 < v.size(); i += 2)
    w.emplace_back(static_cast<float>(v[i]), static_cast<float>(v[i+1]));
  return w;
}

// Wrap GetOptionVector so we can probe *once* without spamming logs
template <typename T>
bool TryGetVector(const YAMLReader& yr, const std::string& dotted_key, std::vector<T>& out) {
  try {
    out = yr.GetOptionVector<T>(dotted_key);
    return true;
  } catch (...) {
    return false;
  }
}

REGISTER_IGUANA_ALGORITHM(RGAFiducialFilter);

// --- env knobs -------------------------------------------------------------

bool RGAFiducialFilter::EnvOn(const char* name) {
  if (const char* s = std::getenv(name)) {
    std::string v{s};
    return (v == "1" || v == "true" || v == "TRUE");
  }
  return false;
}
int RGAFiducialFilter::EnvInt(const char* name, int def) {
  if (const char* s = std::getenv(name)) {
    try { return std::stoi(s); } catch (...) {}
  }
  return def;
}

// --- lifecycle -------------------------------------------------------------

void RGAFiducialFilter::Start(hipo::banklist& banks)
{
  // Debug knobs
  dbg_on     = EnvOn("IGUANA_RGAFID_DEBUG");
  dbg_masks  = EnvOn("IGUANA_RGAFID_DEBUG_MASKS");
  dbg_ft     = EnvOn("IGUANA_RGAFID_DEBUG_FT");
  dbg_events = EnvInt("IGUANA_RGAFID_DEBUG_EVENTS", 0);

  if (dbg_on) {
    m_log->Info("[RGAFID][DEBUG] enabled. masks={}, ft={}, events={}", dbg_masks, dbg_ft, dbg_events);
  }

  // Load YAML (safe if missing)
  ParseYAMLConfig();
  if (dbg_on) m_log->Info("[RGAFID] GetConfig() {}", GetConfig() ? "present" : "null");

  // concurrent params
  o_runnum         = ConcurrentParamFactory::Create<int>();
  o_cal_strictness = ConcurrentParamFactory::Create<int>();

  // strictness precedence: setter > env > YAML > default(1)
  if (!u_strictness_user.has_value()) {
    if (const char* s = std::getenv("IGUANA_RGAFID_STRICTNESS")) {
      try { u_strictness_user = std::clamp(std::stoi(s), 1, 3); } catch (...) {}
    }
  }
  if (!u_strictness_user.has_value()) {
    try {
      auto v = GetOptionVector<int>("calorimeter.strictness");
      if (!v.empty()) u_strictness_user = std::clamp(v.front(), 1, 3);
      if (dbg_on) m_log->Info("[RGAFID] YAML strictness {} (from calorimeter.strictness[0])",
                              v.empty() ? -1 : v.front());
    } catch (...) {
      // quiet: config may omit it
    }
  }
  if (!u_strictness_user.has_value()) u_strictness_user = 1;
  if (dbg_on) m_log->Info("[RGAFID] strictness final = {}", *u_strictness_user);

  // --- FT params from YAML (optional; defaults if missing) ---
  u_ft_params = FTParams{}; // defaults rmin=8.5, rmax=15.5

  if (GetConfig()) {
    std::vector<double> rvec;
    if (TryGetVector(*this, "forward_tagger.radius", rvec) && rvec.size() >= 2) {
      float a = static_cast<float>(rvec[0]);
      float b = static_cast<float>(rvec[1]);
      u_ft_params.rmin = std::min(a, b);
      u_ft_params.rmax = std::max(a, b);
    }

    std::vector<double> holes_flat;
    if (TryGetVector(*this, "forward_tagger.holes_flat", holes_flat)) {
      for (size_t i = 0; i + 2 < holes_flat.size(); i += 3) {
        u_ft_params.holes.push_back({
          static_cast<float>(holes_flat[i]),
          static_cast<float>(holes_flat[i+1]),
          static_cast<float>(holes_flat[i+2])
        });
      }
    }
  }

  if (dbg_on || dbg_ft) DumpFTParams();

  // required banks
  b_particle = GetBankIndex(banks, "REC::Particle");
  b_config   = GetBankIndex(banks, "RUN::config");

  // optional banks
  if (banklist_has(banks, "REC::Calorimeter")) {
    b_calor = GetBankIndex(banks, "REC::Calorimeter");
    m_have_calor = true;
  } else {
    m_have_calor = false;
    m_log->Info("Optional bank 'REC::Calorimeter' not in banklist; calorimeter fiducials will be skipped.");
  }
  if (banklist_has(banks, "REC::ForwardTagger")) {
    b_ft = GetBankIndex(banks, "REC::ForwardTagger");
    m_have_ft = true;
  } else {
    m_have_ft = false;
    m_log->Info("Optional bank 'REC::ForwardTagger' not in banklist; FT fiducials will be skipped.");
  }
}

void RGAFiducialFilter::Run(hipo::banklist& banks) const
{
  auto& particleBank = GetBank(banks, b_particle, "REC::Particle");
  auto& configBank   = GetBank(banks, b_config,   "RUN::config");

  const hipo::bank* calBankPtr = m_have_calor ? &GetBank(banks, b_calor, "REC::Calorimeter") : nullptr;
  const hipo::bank* ftBankPtr  = m_have_ft    ? &GetBank(banks, b_ft,    "REC::ForwardTagger") : nullptr;

  const int runnum = configBank.getInt("run", 0);
  auto key = PrepareEvent(runnum);

  if (dbg_on) {
    static std::atomic<int> once{0};
    if (once.fetch_add(1) == 0) {
      m_log->Info("[RGAFID] Run(): run={} have_calor={} have_ft={} strictness={}",
                  runnum, m_have_calor, m_have_ft, GetCalStrictness(key));
    }
  }

  particleBank.getMutableRowList().filter([this, calBankPtr, ftBankPtr, key](auto, auto row) {
    const int track_index = row;
    const bool accept = Filter(track_index, calBankPtr, ftBankPtr, key);

    if (dbg_on && dbg_events > 0) {
      int seen = ++g_dbg_events_seen;
      if (seen <= dbg_events) {
        m_log->Info("[RGAFID][track={} key={}] -> {}", track_index, (uint64_t)key, accept ? "ACCEPT" : "REJECT");
      }
    }
    return accept ? 1 : 0;
  });
}

void RGAFiducialFilter::Stop() { /* nothing */ }

// --- per-event prep --------------------------------------------------------

concurrent_key_t RGAFiducialFilter::PrepareEvent(int runnum) const
{
  if (o_runnum->NeedsHashing()) {
    std::hash<int> hash_ftn;
    auto key = hash_ftn(runnum);
    if (!o_runnum->HasKey(key)) Reload(runnum, key);
    return key;
  } else {
    if (o_runnum->IsEmpty() || o_runnum->Load(0) != runnum) Reload(runnum, 0);
    return 0;
  }
}

void RGAFiducialFilter::Reload(int runnum, concurrent_key_t key) const
{
  std::lock_guard<std::mutex> lock(m_mutex);

  o_runnum->Save(runnum, key);

  const int strict = std::clamp(u_strictness_user.value_or(1), 1, 3);
  o_cal_strictness->Save(strict, key);

  if (dbg_on) {
    m_log->Info("[RGAFID][Reload] run={} key={} strictness={}", runnum, (uint64_t)key, strict);
  }

  // build masks only once per run; keep *quiet* if config absent
  if (m_masks_by_run.find(runnum) == m_masks_by_run.end()) {
    auto mm = BuildCalMaskCache(runnum);
    if (dbg_on || dbg_masks) DumpMaskSummary(runnum, mm);
    m_masks_by_run.emplace(runnum, std::move(mm));
  }
}

// --- user API --------------------------------------------------------------

void RGAFiducialFilter::SetStrictness(int strictness)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  u_strictness_user = std::clamp(strictness, 1, 3);
}

// --- core filter -----------------------------------------------------------

bool RGAFiducialFilter::Filter(int track_index,
                               const hipo::bank* calBank,
                               const hipo::bank* ftBank,
                               concurrent_key_t key) const
{
  // Calorimeter
  if (calBank != nullptr) {
    CalLayers h = CollectCalHitsForTrack(*calBank, track_index);

    if (h.has_any) {
      const int strictness = GetCalStrictness(key);
      if (!PassCalStrictness(h, strictness)) {
        if (dbg_on && g_dbg_events_seen.load() < dbg_events) {
          m_log->Info("[RGAFID][CAL] track={} strictness={} -> edge FAIL (lv1={}, lw1={})",
                      track_index, strictness, h.lv1, h.lw1);
        }
        return false;
      }

      if (strictness >= 2) {
        if (!PassCalDeadPMTMasks(h, key)) {
          if (dbg_on && g_dbg_events_seen.load() < dbg_events) {
            m_log->Info("[RGAFID][CAL] track={} -> dead-PMT mask FAIL (sec={})",
                        track_index, h.sector);
          }
          return false;
        }
      }
    }
  }

  // Forward Tagger
  if (!PassFTFiducial(track_index, ftBank)) {
    if (dbg_on && g_dbg_events_seen.load() < dbg_events) {
      m_log->Info("[RGAFID][FT] track={} -> FT FAIL", track_index);
    }
    return false;
  }

  return true;
}

// --- helpers ---------------------------------------------------------------

RGAFiducialFilter::CalLayers
RGAFiducialFilter::CollectCalHitsForTrack(const hipo::bank& calBank, int pindex)
{
  CalLayers out;
  const int nrows = calBank.getRows();
  for (int i = 0; i < nrows; ++i) {
    if (calBank.getInt("pindex", i) != pindex) continue;

    out.has_any = true;
    out.sector  = calBank.getInt("sector", i);
    const int layer = calBank.getInt("layer", i);

    const float lv = calBank.getFloat("lv", i);
    const float lw = calBank.getFloat("lw", i);
    const float lu = calBank.getFloat("lu", i);

    if      (layer == 1) { out.lv1 = lv; out.lw1 = lw; out.lu1 = lu; }
    else if (layer == 4) { out.lv4 = lv; out.lw4 = lw; out.lu4 = lu; }
    else if (layer == 7) { out.lv7 = lv; out.lw7 = lw; out.lu7 = lu; }
  }
  return out;
}

bool RGAFiducialFilter::PassCalStrictness(const CalLayers& h, int strictness)
{
  switch (strictness) {
    case 1: if (h.lw1 <  9.0f || h.lv1 <  9.0f) return false; break;
    case 2: if (h.lw1 < 13.5f || h.lv1 < 13.5f) return false; break;
    case 3: if (h.lw1 < 18.0f || h.lv1 < 18.0f) return false; break;
    default: return false;
  }
  return true;
}

// BuildCalMaskCache with *minimal* probing to avoid log spam
RGAFiducialFilter::MaskMap RGAFiducialFilter::BuildCalMaskCache(int runnum) const
{
  MaskMap out;
  if (!GetConfig()) return out;

  // 1) Pick masks block index by checking a small range (0..3 by default)
  const int idx_max = EnvInt("IGUANA_RGAFID_MASK_INDEX_SCAN_MAX", 3);
  int idx = -1;

  for (int i = 0; i <= idx_max; ++i) {
    std::vector<int> rr;
    if (TryGetVector(*this, "calorimeter.masks." + std::to_string(i) + ".runs", rr)) {
      if (rr.size() >= 2 && runnum >= rr.front() && runnum <= rr.back()) { idx = i; break; }
    }
  }
  if (idx < 0) {
    // Fallback: first index with no runs gate (probe once)
    for (int i = 0; i <= idx_max; ++i) {
      std::vector<int> rr;
      if (!TryGetVector(*this, "calorimeter.masks." + std::to_string(i) + ".runs", rr)) {
        idx = i; break;
      }
    }
  }
  if (idx < 0) {
    if (dbg_on || dbg_masks) m_log->Info("[RGAFID][MASK] no usable masks entry; run={} -> using empty masks", runnum);
    return out;
  }

  if (dbg_on || dbg_masks) m_log->Info("[RGAFID][MASK] run={} -> masks index={}", runnum, idx);

  // 2) Sentinel probe: if sector-1 PCAL lv cal_mask missing, assume no masks at all.
  {
    std::vector<double> sentinel;
    const std::string k = "calorimeter.masks." + std::to_string(idx) + ".sectors.1.pcal.lv.cal_mask";
    if (!TryGetVector(*this, k, sentinel)) {
      if (dbg_on || dbg_masks) m_log->Info("[RGAFID][MASK] no sector/axis windows found at {}, skipping masks", k);
      return out;
    }
    // If present, we’ll still read *all* sectors/axes below; this probe avoided 50+ error lines.
  }

  auto read_axis = [this, idx](int sector, const char* layer, const char* axis) -> std::vector<window_t> {
    std::vector<double> flat;
    const std::string key = "calorimeter.masks." + std::to_string(idx) +
                            ".sectors." + std::to_string(sector) + "." + layer + "." + axis + ".cal_mask";
    if (!TryGetVector(*this, key, flat)) return {};
    return to_windows_flat(flat);
  };

  for (int s = 1; s <= 6; ++s) {
    SectorMasks sm;
    sm.pcal.lv  = read_axis(s, "pcal",  "lv");
    sm.pcal.lw  = read_axis(s, "pcal",  "lw");
    sm.pcal.lu  = read_axis(s, "pcal",  "lu");
    sm.ecin.lv  = read_axis(s, "ecin",  "lv");
    sm.ecin.lw  = read_axis(s, "ecin",  "lw");
    sm.ecin.lu  = read_axis(s, "ecin",  "lu");
    sm.ecout.lv = read_axis(s, "ecout", "lv");
    sm.ecout.lw = read_axis(s, "ecout", "lw");
    sm.ecout.lu = read_axis(s, "ecout", "lu");
    out.emplace(s, std::move(sm));
  }

  if (dbg_on || dbg_masks) DumpMaskSummary(runnum, out);
  return out;
}

bool RGAFiducialFilter::PassCalDeadPMTMasks(const CalLayers& h, concurrent_key_t key) const
{
  const int runnum = GetRunNum(key);

  std::lock_guard<std::mutex> lock(m_mutex);

  auto it = m_masks_by_run.find(runnum);
  if (it == m_masks_by_run.end())
    it = m_masks_by_run.emplace(runnum, BuildCalMaskCache(runnum)).first;

  const auto& m = it->second;
  auto itsec = m.find(h.sector);
  if (itsec == m.end()) return true;
  const auto& sm = itsec->second;

  auto in_any = [](float v, const std::vector<window_t>& wins){
    for (auto const& w : wins) if (v > w.first && v < w.second) return true;
    return false;
  };

  const bool fail =
    in_any(h.lv1, sm.pcal.lv) || in_any(h.lw1, sm.pcal.lw) || in_any(h.lu1, sm.pcal.lu) ||
    in_any(h.lv4, sm.ecin.lv) || in_any(h.lw4, sm.ecin.lw) || in_any(h.lu4, sm.ecin.lu) ||
    in_any(h.lv7, sm.ecout.lv)|| in_any(h.lw7, sm.ecout.lw)|| in_any(h.lu7, sm.ecout.lu);

  if ((dbg_on || dbg_masks) && fail && g_dbg_events_seen.load() < dbg_events) {
    m_log->Info("[RGAFID][MASK] sec={} (lv1,lw1,lu1)=({:.1f},{:.1f},{:.1f})"
                " (lv4,lw4,lu4)=({:.1f},{:.1f},{:.1f})"
                " (lv7,lw7,lu7)=({:.1f},{:.1f},{:.1f}) -> FAIL",
                h.sector,
                h.lv1,h.lw1,h.lu1, h.lv4,h.lw4,h.lu4, h.lv7,h.lw7,h.lu7);
  }

  return !fail;
}

bool RGAFiducialFilter::PassFTFiducial(int track_index, const hipo::bank* ftBank) const
{
  if (ftBank == nullptr) return true;

  const int nrows = ftBank->getRows();
  for (int i = 0; i < nrows; ++i) {
    if (ftBank->getInt("pindex", i) != track_index) continue;

    const double x = ftBank->getFloat("x", i);
    const double y = ftBank->getFloat("y", i);
    const double r = std::sqrt(x*x + y*y);

    if (dbg_ft && g_dbg_events_seen.load() < std::max(1, dbg_events)) {
      m_log->Info("[RGAFID][FT] track={} x={:.2f} y={:.2f} r={:.2f} rwin=[{:.2f},{:.2f}]",
                  track_index, x, y, r, u_ft_params.rmin, u_ft_params.rmax);
    }

    if (r < u_ft_params.rmin) return false;
    if (r > u_ft_params.rmax) return false;

    for (auto const& h : u_ft_params.holes) {
      const double d = std::sqrt((x - h[1])*(x - h[1]) + (y - h[2])*(y - h[2]));
      if (d < h[0]) {
        if (dbg_ft && g_dbg_events_seen.load() < std::max(1, dbg_events)) {
          m_log->Info("[RGAFID][FT] track={} inside hole R={:.2f} @({:.2f},{:.2f})",
                      track_index, h[0], h[1], h[2]);
        }
        return false;
      }
    }

    return true; // first associated FT row decides
  }

  return true; // no FT association -> pass
}

// --- accessors -------------------------------------------------------------

int RGAFiducialFilter::GetRunNum(concurrent_key_t key) const { return o_runnum->Load(key); }
int RGAFiducialFilter::GetCalStrictness(concurrent_key_t key) const { return o_cal_strictness->Load(key); }

// --- debug dumps -----------------------------------------------------------

void RGAFiducialFilter::DumpFTParams() const {
  m_log->Info("[RGAFID][FT] params: rmin={:.3f} rmax={:.3f} holes={}",
              u_ft_params.rmin, u_ft_params.rmax, u_ft_params.holes.size());
  for (size_t i = 0; i < u_ft_params.holes.size() && i < 8; ++i) {
    const auto& h = u_ft_params.holes[i];
    m_log->Info("   hole[{}] R={:.3f} cx={:.3f} cy={:.3f}", i, h[0], h[1], h[2]);
  }
}

void RGAFiducialFilter::DumpMaskSummary(int runnum, const MaskMap& mm) const {
  size_t total = 0;
  auto sumv = [&](const std::vector<window_t>& v){ total += v.size(); };
  for (const auto& [sec, sm] : mm) {
    (void)sec;
    sumv(sm.pcal.lv); sumv(sm.pcal.lw); sumv(sm.pcal.lu);
    sumv(sm.ecin.lv); sumv(sm.ecin.lw); sumv(sm.ecin.lu);
    sumv(sm.ecout.lv);sumv(sm.ecout.lw);sumv(sm.ecout.lu);
  }
  m_log->Info("[RGAFID][MASK] run={} sectors={} total_windows={}", runnum, mm.size(), total);
}

} // namespace iguana::clas12