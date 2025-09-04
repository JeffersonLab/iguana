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

  // Load YAML (safe if missing). Only consulted if IGUANA_RGAFID_USE_YAML=1.
  ParseYAMLConfig();
  const bool use_yaml = EnvOn("IGUANA_RGAFID_USE_YAML");
  if (dbg_on) m_log->Info("[RGAFID] GetConfig() {} (use_yaml={})",
                          GetConfig() ? "present" : "null", use_yaml);

  // concurrent params
  o_runnum         = ConcurrentParamFactory::Create<int>();
  o_cal_strictness = ConcurrentParamFactory::Create<int>();

  // strictness precedence: env > YAML (if enabled) > default(1)
  if (!u_strictness_user.has_value()) {
    if (const char* s = std::getenv("IGUANA_RGAFID_STRICTNESS")) {
      try { u_strictness_user = std::clamp(std::stoi(s), 1, 3); } catch (...) {}
    }
  }
  if (!u_strictness_user.has_value() && use_yaml) {
    try {
      auto v = GetOptionVector<int>("calorimeter.strictness");
      if (!v.empty()) u_strictness_user = std::clamp(v.front(), 1, 3);
      if (dbg_on) m_log->Info("[RGAFID] YAML strictness {} (from calorimeter.strictness[0])",
                              v.empty() ? -1 : v.front());
    } catch (...) { /* quiet */ }
  }
  if (!u_strictness_user.has_value()) u_strictness_user = 1;
  if (dbg_on) m_log->Info("[RGAFID] strictness final = {}", *u_strictness_user);

  // --- FT params (defaults; YAML override only if explicitly enabled) ---
  u_ft_params = FTParams{}; // defaults rmin=8.5, rmax=15.5
  {
    // Built-in hole list (radius, cx, cy) in cm
    const float H[] = {
      1.60f, -8.42f,  9.89f,
      1.60f, -9.89f, -5.33f,
      2.30f, -6.15f, -13.00f,
      2.00f,  3.70f, -6.50f
    };
    for (size_t i = 0; i + 2 < std::size(H); i += 3)
      u_ft_params.holes.push_back({H[i], H[i+1], H[i+2]});
  }

  if (use_yaml && GetConfig()) {
    // Optional FT radius override
    try {
      auto rvec = GetOptionVector<double>("forward_tagger.radius");
      if (rvec.size() >= 2) {
        float a = static_cast<float>(rvec[0]);
        float b = static_cast<float>(rvec[1]);
        u_ft_params.rmin = std::min(a, b);
        u_ft_params.rmax = std::max(a, b);
      }
    } catch (...) {}
    // Optional FT holes override
    try {
      auto flat = GetOptionVector<double>("forward_tagger.holes_flat");
      if (!flat.empty()) {
        u_ft_params.holes.clear();
        for (size_t i = 0; i + 2 < flat.size(); i += 3) {
          u_ft_params.holes.push_back({
            static_cast<float>(flat[i]),
            static_cast<float>(flat[i+1]),
            static_cast<float>(flat[i+2])
          });
        }
      }
    } catch (...) {}
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

  // build masks only once per run
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
          // print representative minima for context
          auto minv = [](const std::vector<float>& v){ return v.empty()?0.f:*std::min_element(v.begin(), v.end()); };
          m_log->Info("[RGAFID][CAL] track={} strictness={} -> edge FAIL (min lv1={}, min lw1={})",
                      track_index, strictness, minv(h.L1.lv), minv(h.L1.lw));
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
    const int sector = calBank.getInt("sector", i);
    if (out.sector == 0 && sector >= 1 && sector <= 6) out.sector = sector;

    const int layer = calBank.getInt("layer", i);
    const float lv = calBank.getFloat("lv", i);
    const float lw = calBank.getFloat("lw", i);
    const float lu = calBank.getFloat("lu", i);

    if      (layer == 1) { out.L1.lv.push_back(lv); out.L1.lw.push_back(lw); out.L1.lu.push_back(lu); }
    else if (layer == 4) { out.L4.lv.push_back(lv); out.L4.lw.push_back(lw); out.L4.lu.push_back(lu); }
    else if (layer == 7) { out.L7.lv.push_back(lv); out.L7.lw.push_back(lw); out.L7.lu.push_back(lu); }
  }
  return out;
}

static inline bool value_in_any_window(float v, const std::vector<std::pair<float,float>>& wins)
{
  for (auto const& w : wins) if (v > w.first && v < w.second) return true;
  return false;
}

bool RGAFiducialFilter::PassCalStrictness(const CalLayers& h, int strictness)
{
  // Strictness is a PCAL-edge cut applied to ALL PCAL hits: we use minima.
  if (h.L1.lv.empty() || h.L1.lw.empty()) return true; // no PCAL -> do not apply

  const float min_lv = *std::min_element(h.L1.lv.begin(), h.L1.lv.end());
  const float min_lw = *std::min_element(h.L1.lw.begin(), h.L1.lw.end());

  switch (strictness) {
    case 1: return !(min_lw <  9.0f || min_lv <  9.0f);
    case 2: return !(min_lw < 13.5f || min_lv < 13.5f);
    case 3: return !(min_lw < 18.0f || min_lv < 18.0f);
    default: return false;
  }
}

// BuildCalMaskCache: hard-coded defaults; optional YAML replacement if enabled
RGAFiducialFilter::MaskMap RGAFiducialFilter::BuildCalMaskCache(int /*runnum*/) const
{
  MaskMap out;

  auto add = [&](int sec, SectorMasks& sm,
                 const std::vector<window_t>& p_lv,
                 const std::vector<window_t>& p_lw,
                 const std::vector<window_t>& p_lu,
                 const std::vector<window_t>& i_lv = {},
                 const std::vector<window_t>& i_lw = {},
                 const std::vector<window_t>& i_lu = {},
                 const std::vector<window_t>& o_lv = {},
                 const std::vector<window_t>& o_lw = {},
                 const std::vector<window_t>& o_lu = {}) {
    sm.pcal.lv  = p_lv; sm.pcal.lw  = p_lw; sm.pcal.lu  = p_lu;
    sm.ecin.lv  = i_lv; sm.ecin.lw  = i_lw; sm.ecin.lu  = i_lu;
    sm.ecout.lv = o_lv; sm.ecout.lw = o_lw; sm.ecout.lu = o_lu;
    out.emplace(sec, std::move(sm));
  };

  // ---------- Built-in masks (in cm) ----------
  {
    SectorMasks s1,s2,s3,s4,s5,s6;
    add(1, s1,
        /*PCAL*/ std::vector<window_t>{},                 // lv
                 std::vector<window_t>{{72.0f,94.5f},{220.5f,234.0f}}, // lw
                 std::vector<window_t>{},                 // lu
        /*ECIN*/ std::vector<window_t>{{67.5f,94.5f}}, {}, {},
        /*ECOUT*/std::vector<window_t>{{0.0f,40.5f}}, {}, {});
    add(2, s2,
        /*PCAL*/ std::vector<window_t>{{99.0f,117.0f}}, {}, {});
    add(3, s3,
        /*PCAL*/ std::vector<window_t>{}, std::vector<window_t>{{346.5f,378.0f}}, {});
    add(4, s4,
        /*PCAL*/ std::vector<window_t>{{229.5f,243.0f}},
                 std::vector<window_t>{{0.0f,13.5f}}, {});
    add(5, s5,
        /*PCAL*/ std::vector<window_t>{}, {}, {},
        /*ECIN*/ std::vector<window_t>{{0.0f,23.5f}}, {}, {},
        /*ECOUT*/std::vector<window_t>{}, {}, std::vector<window_t>{{193.5f,216.0f}});
    add(6, s6,
        /*PCAL*/ std::vector<window_t>{}, std::vector<window_t>{{166.5f,193.5f}}, {});
  }

  // ---------- Optional YAML override (opt-in) ----------
  if (EnvOn("IGUANA_RGAFID_USE_YAML") && GetConfig()) {
    MaskMap yaml;
    auto read_axis = [this](const std::string& key, std::vector<window_t>& dst) {
      try {
        auto flat = GetOptionVector<double>(key);
        dst.clear();
        for (size_t i=0;i+1<flat.size();i+=2) dst.emplace_back((float)flat[i], (float)flat[i+1]);
        return true;
      } catch (...) { return false; }
    };

    for (int s=1; s<=6; ++s) {
      SectorMasks sm;
      const std::string base = "calorimeter.masks.0.sectors." + std::to_string(s) + ".";
      (void)read_axis(base+"pcal.lv.cal_mask",  sm.pcal.lv);
      (void)read_axis(base+"pcal.lw.cal_mask",  sm.pcal.lw);
      (void)read_axis(base+"pcal.lu.cal_mask",  sm.pcal.lu);
      (void)read_axis(base+"ecin.lv.cal_mask",  sm.ecin.lv);
      (void)read_axis(base+"ecin.lw.cal_mask",  sm.ecin.lw);
      (void)read_axis(base+"ecin.lu.cal_mask",  sm.ecin.lu);
      (void)read_axis(base+"ecout.lv.cal_mask", sm.ecout.lv);
      (void)read_axis(base+"ecout.lw.cal_mask", sm.ecout.lw);
      (void)read_axis(base+"ecout.lu.cal_mask", sm.ecout.lu);
      yaml.emplace(s, std::move(sm));
    }
    out.swap(yaml);
  }

  if (dbg_on || dbg_masks) DumpMaskSummary(-1, out);
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

  auto any_in = [](const std::vector<float>& vals, const std::vector<window_t>& wins){
    for (float v : vals) if (value_in_any_window(v, wins)) return true;
    return false;
  };

  const bool fail =
    any_in(h.L1.lv, sm.pcal.lv) || any_in(h.L1.lw, sm.pcal.lw) || any_in(h.L1.lu, sm.pcal.lu) ||
    any_in(h.L4.lv, sm.ecin.lv) || any_in(h.L4.lw, sm.ecin.lw) || any_in(h.L4.lu, sm.ecin.lu) ||
    any_in(h.L7.lv, sm.ecout.lv)|| any_in(h.L7.lw, sm.ecout.lw)|| any_in(h.L7.lu, sm.ecout.lu);

  if ((dbg_on || dbg_masks) && fail && g_dbg_events_seen.load() < dbg_events) {
    auto pick = [](const std::vector<float>& v){ return v.empty()?0.f:v.front(); };
    m_log->Info("[RGAFID][MASK] sec={} (ex. lv1,lw1,lu1)=({:.1f},{:.1f},{:.1f}) -> FAIL",
                h.sector, pick(h.L1.lv), pick(h.L1.lw), pick(h.L1.lu));
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