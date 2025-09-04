#include "Algorithm.h"
#include "iguana/algorithms/TypeDefs.h"
#include "iguana/services/YAMLReader.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace iguana::clas12 {

// helper: does the banklist the framework gave us include a bank with this name?
static bool banklist_has(hipo::banklist& banks, const char* name) {
  for (auto& b : banks) if (b.getSchema().getName() == name) return true;
  return false;
}

// turn a flat vector into [a,b] windows
static std::vector<std::pair<float,float>>
to_windows_flat(const std::vector<double>& v) {
  std::vector<std::pair<float,float>> w;
  w.reserve(v.size() / 2);
  for (size_t i = 0; i + 1 < v.size(); i += 2)
    w.emplace_back(static_cast<float>(v[i]), static_cast<float>(v[i+1]));
  return w;
}

REGISTER_IGUANA_ALGORITHM(RGAFiducialFilter);

// -------------------------
// tiny env helpers
// -------------------------
bool RGAFiducialFilter::EnvOn(const char* name) {
  if (const char* s = std::getenv(name)) {
    return (std::string(s) == "1" || std::string(s) == "true" || std::string(s) == "TRUE");
  }
  return false;
}
int RGAFiducialFilter::EnvInt(const char* name, int def) {
  if (const char* s = std::getenv(name)) {
    try { return std::stoi(s); } catch (...) {}
  }
  return def;
}

// -------------------------
// lifecycle
// -------------------------

void RGAFiducialFilter::Start(hipo::banklist& banks)
{
  // Debug knobs (read once)
  dbg_on     = EnvOn("IGUANA_RGAFID_DEBUG");
  dbg_masks  = EnvOn("IGUANA_RGAFID_DEBUG_MASKS");
  dbg_ft     = EnvOn("IGUANA_RGAFID_DEBUG_FT");
  dbg_events = EnvInt("IGUANA_RGAFID_DEBUG_EVENTS", 0);

  if (dbg_on) {
    m_log->Info("[RGAFID][DEBUG] enabled. masks={}, ft={}, events={}",
                dbg_masks, dbg_ft, dbg_events);
  }

  // Load YAML (safe if missing)
  ParseYAMLConfig();
  if (dbg_on) {
    m_log->Info("[RGAFID] GetConfig() {}", GetConfig() ? "present" : "null");
  }

  // thread-safe params
  o_runnum         = ConcurrentParamFactory::Create<int>();
  o_cal_strictness = ConcurrentParamFactory::Create<int>();

  // --- strictness precedence: user setter > env var > YAML > default(1) ---
  if (!u_strictness_user.has_value()) {
    if (const char* s = std::getenv("IGUANA_RGAFID_STRICTNESS")) {
      try { u_strictness_user = std::clamp(std::stoi(s), 1, 3); } catch (...) {}
    }
  }
  if (!u_strictness_user.has_value()) {
    try {
      // IMPORTANT: key is the leaf, path is the parent
      auto v = GetOptionVector<int>("strictness",
                                    YAMLReader::node_path_t{ "calorimeter" });
      if (!v.empty()) u_strictness_user = std::clamp(v.front(), 1, 3);
      if (dbg_on) m_log->Info("[RGAFID] YAML strictness {} (from calorimeter.strictness[0])",
                              v.empty() ? -1 : v.front());
    } catch (const std::exception& e) {
      if (dbg_on) m_log->Info("[RGAFID] strictness YAML read failed: {}", e.what());
    }
  }
  if (!u_strictness_user.has_value()) u_strictness_user = 1;
  if (dbg_on) m_log->Info("[RGAFID] strictness final = {}", *u_strictness_user);

  // Forward Tagger parameters from YAML (optional; defaults if missing)
  u_ft_params = FTParams{}; // default rmin=8.5, rmax=15.5, empty holes

  // radius: parent path "forward_tagger", key "radius"
  try {
    auto r = GetOptionVector<double>("radius",
                                     YAMLReader::node_path_t{ "forward_tagger" });
    if (r.size() >= 2) {
      float a = static_cast<float>(r[0]);
      float b = static_cast<float>(r[1]);
      u_ft_params.rmin = std::min(a, b);
      u_ft_params.rmax = std::max(a, b);
    }
    if (dbg_on || dbg_ft) m_log->Info("[RGAFID][FT] radius read size={} -> rmin={}, rmax={}",
                                      r.size(), u_ft_params.rmin, u_ft_params.rmax);
  } catch (const std::exception& e) {
    if (dbg_on || dbg_ft) m_log->Info("[RGAFID][FT] radius YAML read failed: {}", e.what());
  }

  // holes_flat: parent path "forward_tagger", key "holes_flat"
  try {
    auto flat = GetOptionVector<double>("holes_flat",
                                        YAMLReader::node_path_t{ "forward_tagger" });
    if (dbg_on || dbg_ft) m_log->Info("[RGAFID][FT] holes_flat size={}", flat.size());
    for (size_t i = 0; i + 2 < flat.size(); i += 3) {
      u_ft_params.holes.push_back({
        static_cast<float>(flat[i]),
        static_cast<float>(flat[i+1]),
        static_cast<float>(flat[i+2])
      });
      if ((dbg_on || dbg_ft) && i < 12) {
        m_log->Info("[RGAFID][FT] hole triplet[{}] = ({:.3f},{:.3f},{:.3f})",
                    i/3, flat[i], flat[i+1], flat[i+2]);
      }
    }
  } catch (const std::exception& e) {
    if (dbg_on || dbg_ft) m_log->Info("[RGAFID][FT] holes_flat YAML read failed: {}", e.what());
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

  // Optional banks (nullptr => skip those cuts)
  const hipo::bank* calBankPtr = m_have_calor ? &GetBank(banks, b_calor, "REC::Calorimeter") : nullptr;
  const hipo::bank* ftBankPtr  = m_have_ft    ? &GetBank(banks, b_ft,    "REC::ForwardTagger") : nullptr;

  const int runnum = configBank.getInt("run", 0);
  auto key = PrepareEvent(runnum);

  if (dbg_on) {
    static std::atomic<int> once {0};
    if (once.fetch_add(1) == 0) {
      m_log->Info("[RGAFID] Run(): run={} have_calor={} have_ft={} strictness={}",
                  runnum, m_have_calor, m_have_ft, GetCalStrictness(key));
    }
  }

  // filter tracks in place
  particleBank.getMutableRowList().filter([this, calBankPtr, ftBankPtr, key](auto /*bank*/, auto row) {
    const int track_index = row;
    const bool accept = Filter(track_index, calBankPtr, ftBankPtr, key);

    if (dbg_on && dbg_events > 0) {
      int seen = ++dbg_events_seen;
      if (seen <= dbg_events) {
        m_log->Info("[RGAFID][track={} key={}] -> {}", track_index, (uint64_t)key, accept ? "ACCEPT" : "REJECT");
      }
    }
    return accept ? 1 : 0;
  });
}

void RGAFiducialFilter::Stop() { /* nothing */ }

// -------------------------
// event preparation
// -------------------------

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
  std::lock_guard<std::mutex> const lock(m_mutex);

  o_runnum->Save(runnum, key);

  // calorimeter strictness (from user/env/YAML -> clamped)
  const int strict = std::clamp(u_strictness_user.value_or(1), 1, 3);
  o_cal_strictness->Save(strict, key);

  if (dbg_on) {
    m_log->Info("[RGAFID][Reload] run={} key={} strictness={}", runnum, (uint64_t)key, strict);
  }

  // build and cache masks per run (from YAML only)
  if (m_masks_by_run.find(runnum) == m_masks_by_run.end()) {
    auto mm = BuildCalMaskCache(runnum);
    if (dbg_on || dbg_masks) DumpMaskSummary(runnum, mm);
    m_masks_by_run.emplace(runnum, std::move(mm));
  }
}

// -------------------------
// user setter
// -------------------------

void RGAFiducialFilter::SetStrictness(int strictness)
{
  std::lock_guard<std::mutex> const lock(m_mutex);
  u_strictness_user = std::clamp(strictness, 1, 3);
}

// -------------------------
// core filter
// -------------------------

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
        if (dbg_on && dbg_events_seen.load() < dbg_events) {
          m_log->Info("[RGAFID][CAL] track={} strictness={} -> edge FAIL (lv1={}, lw1={})",
                      track_index, strictness, h.lv1, h.lw1);
        }
        return false;
      }

      if (strictness >= 2) {
        if (!PassCalDeadPMTMasks(h, key)) {
          if (dbg_on && dbg_events_seen.load() < dbg_events) {
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
    if (dbg_on && dbg_events_seen.load() < dbg_events) {
      m_log->Info("[RGAFID][FT] track={} -> FT FAIL", track_index);
    }
    return false;
  }

  return true;
}

// -------------------------
// helpers
// -------------------------

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

RGAFiducialFilter::MaskMap RGAFiducialFilter::BuildCalMaskCache(int runnum) const
{
  MaskMap out;
  if (!GetConfig()) {
    if (dbg_on || dbg_masks) m_log->Info("[RGAFID][MASK] no YAML config; empty masks");
    return out; // no YAML -> no dead-PMT masks
  }

  auto read_axis = [this, runnum](int sector, const char* layer, const char* axis) -> std::vector<window_t> {
    YAMLReader::node_path_t p;

    // Choose run-range element or default
    bool used_default = false;
    if (runnum > 0) {
      try {
        p = { "calorimeter", "masks",
              GetConfig()->InRange("runs", runnum), // selects the element within masks
              "sectors", std::to_string(sector), layer, axis };
      } catch (...) {
        p = { "calorimeter", "masks", "default",
              "sectors", std::to_string(sector), layer, axis };
        used_default = true;
      }
    } else {
      p = { "calorimeter", "masks", "default",
            "sectors", std::to_string(sector), layer, axis };
      used_default = true;
    }

    if (dbg_on || dbg_masks) {
      m_log->Info("[RGAFID][MASK] path={} {} sec={} layer={} axis={}",
                  used_default ? "default" : "range", used_default ? "(fallback)" : "",
                  sector, layer, axis);
    }

    try {
      // IMPORTANT: key is the leaf at this parent path
      auto flat = GetOptionVector<double>("cal_mask", p);
      if ((dbg_on || dbg_masks) && !flat.empty()) {
        m_log->Info("[RGAFID][MASK] cal_mask size={} first={:.3f}", flat.size(), flat.front());
      }
      return to_windows_flat(flat);
    } catch (const std::exception& e) {
      if (dbg_on || dbg_masks) {
        m_log->Info("[RGAFID][MASK] cal_mask read EXC: {}", e.what());
      }
      return {};
    }
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
  return out;
}

bool RGAFiducialFilter::PassCalDeadPMTMasks(const CalLayers& h, concurrent_key_t key) const
{
  const int runnum = GetRunNum(key);

  // Guard accesses to m_masks_by_run for thread-safety w.r.t. Reload()
  std::lock_guard<std::mutex> const lock(m_mutex);

  auto it = m_masks_by_run.find(runnum);
  if (it == m_masks_by_run.end()) {
    // Should not happen if Reload() ran, but be defensive.
    it = m_masks_by_run.emplace(runnum, BuildCalMaskCache(runnum)).first;
  }
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

  if ((dbg_on || dbg_masks) && fail && dbg_events_seen.load() < dbg_events) {
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

    if (dbg_ft && dbg_events_seen.load() < std::max(1, dbg_events)) {
      m_log->Info("[RGAFID][FT] track={} x={:.2f} y={:.2f} r={:.2f} rwin=[{:.2f},{:.2f}]",
                  track_index, x, y, r, u_ft_params.rmin, u_ft_params.rmax);
    }

    if (r < u_ft_params.rmin) return false;
    if (r > u_ft_params.rmax) return false;

    for (auto const& h : u_ft_params.holes) {
      const double d = std::sqrt((x - h[1])*(x - h[1]) + (y - h[2])*(y - h[2]));
      if (d < h[0]) {
        if (dbg_ft && dbg_events_seen.load() < std::max(1, dbg_events)) {
          m_log->Info("[RGAFID][FT] track={} inside hole R={:.2f} @({:.2f},{:.2f})",
                      track_index, h[0], h[1], h[2]);
        }
        return false;
      }
    }

    return true; // associated FT row passes
  }

  // No FT association for this track in this event -> pass-through
  return true;
}

// -------------------------
// accessors
// -------------------------

int RGAFiducialFilter::GetRunNum(concurrent_key_t key) const
{
  return o_runnum->Load(key);
}

int RGAFiducialFilter::GetCalStrictness(concurrent_key_t key) const
{
  return o_cal_strictness->Load(key);
}

// -------------------------
// debug dumps
// -------------------------
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
  m_log->Info("[RGAFID][MASK] run={} sectors={} total_windows={}",
              runnum, mm.size(), total);
}

} // namespace iguana::clas12