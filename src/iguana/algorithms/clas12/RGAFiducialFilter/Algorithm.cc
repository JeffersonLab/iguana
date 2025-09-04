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
std::atomic<int> g_dbg_events_seen{0};   // TU-local: avoids touching the header
}

namespace iguana::clas12 {

// ==========================================================================
// Small helpers (local, simple)
// ==========================================================================

static bool banklist_has(hipo::banklist& banks, const char* name) {
  for (auto& b : banks) if (b.getSchema().getName() == name) return true;
  return false;
}

// Read vector from this algorithm's YAML block; be QUIET if missing.
template <typename Reader, typename T>
static bool TryGetVector(const Reader& r, const std::string& dotted_key, std::vector<T>& out) {
  try {
    out = r.template GetOptionVector<T>(dotted_key);
    return true;
  } catch (...) {
    return false;
  }
}

// Axis selector for masks
enum Axis : int { AX_LV = 0, AX_LW = 1, AX_LU = 2 };

// ==========================================================================
// Registration
// ==========================================================================
REGISTER_IGUANA_ALGORITHM(RGAFiducialFilter);

// ==========================================================================
// Environment toggles (tiny, local)
// ==========================================================================
bool RGAFiducialFilter::EnvOn(const char* name) {
  if (const char* s = std::getenv(name)) {
    std::string v{s};
    return (v == "1" || v == "true" || v == "TRUE" || v == "on" || v == "ON");
  }
  return false;
}
int RGAFiducialFilter::EnvInt(const char* name, int def) {
  if (const char* s = std::getenv(name)) {
    try { return std::stoi(s); } catch (...) {}
  }
  return def;
}

// ==========================================================================
// Lifecycle
// ==========================================================================
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

  // Thread-friendly params (we still keep these to match the header/API)
  o_runnum         = ConcurrentParamFactory::Create<int>();
  o_cal_strictness = ConcurrentParamFactory::Create<int>();

  // ------------------------------------------------------------------------
  // Strictness (simple): env > YAML:strictness (> YAML legacy) > default(1)
  // ------------------------------------------------------------------------
  if (!u_strictness_user.has_value()) {
    if (const char* s = std::getenv("IGUANA_RGAFID_STRICTNESS")) {
      try { u_strictness_user = std::clamp(std::stoi(s), 1, 3); } catch (...) {}
    }
  }
  if (!u_strictness_user.has_value()) {
    std::vector<int> v;
    if (TryGetVector(*this, "strictness", v) && !v.empty())
      u_strictness_user = std::clamp(v.front(), 1, 3);
    else if (TryGetVector(*this, "calorimeter.strictness", v) && !v.empty())
      u_strictness_user = std::clamp(v.front(), 1, 3);
  }
  if (!u_strictness_user.has_value()) u_strictness_user = 1;
  if (dbg_on) m_log->Info("[RGAFID] strictness final = {}", *u_strictness_user);

  // ------------------------------------------------------------------------
  // Forward Tagger parameters (simple: radius, holes). Defaults baked in.
  // YAML keys (either):  ft_radius, ft_holes_flat
  //                      forward_tagger.radius, forward_tagger.holes_flat
  // ------------------------------------------------------------------------
  u_ft_params = FTParams{}; // defaults rmin=8.5, rmax=15.5, no holes

  // Default holes (so FT holes work even with empty YAML)
  {
    const float H[] = {
      1.60f, -8.42f,  9.89f,
      1.60f, -9.89f, -5.33f,
      2.30f, -6.15f, -13.00f,
      2.00f,  3.70f, -6.50f
    };
    for (size_t i = 0; i + 2 < std::size(H); i += 3)
      u_ft_params.holes.push_back({H[i], H[i+1], H[i+2]});
  }

  // YAML override radius
  {
    std::vector<double> rvec;
    if ((!TryGetVector(*this, "ft_radius", rvec) &&
         !TryGetVector(*this, "forward_tagger.radius", rvec)) == false &&
        rvec.size() >= 2) {
      float a = static_cast<float>(rvec[0]);
      float b = static_cast<float>(rvec[1]);
      u_ft_params.rmin = std::min(a, b);
      u_ft_params.rmax = std::max(a, b);
    }
  }

  // YAML override holes
  {
    std::vector<double> flat;
    if ((!TryGetVector(*this, "ft_holes_flat", flat) &&
         !TryGetVector(*this, "forward_tagger.holes_flat", flat)) == false) {
      u_ft_params.holes.clear();
      for (size_t i = 0; i + 2 < flat.size(); i += 3) {
        u_ft_params.holes.push_back({
          static_cast<float>(flat[i+0]),
          static_cast<float>(flat[i+1]),
          static_cast<float>(flat[i+2])
        });
      }
    }
  }

  if (dbg_on || dbg_ft) DumpFTParams();

  // ------------------------------------------------------------------------
  // Build a single, global mask map once (no per-run scanning).
  // YAML key: cal_masks_flat = [sec, layer, axis, a, b, sec, layer, axis, a, b, ...]
  //            layer: 1=PCAL, 4=ECIN, 7=ECOUT; axis: 0=lv, 1=lw, 2=lu
  // If YAML is missing/empty, we use baked-in defaults (the ones you listed).
  // ------------------------------------------------------------------------
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_masks_by_run.clear(); // (we keep type but only use the current run key)
    const int pseudo_run = -1;
    m_masks_by_run.emplace(pseudo_run, BuildCalMaskCache(pseudo_run));
    if (dbg_on || dbg_masks) DumpMaskSummary(pseudo_run, m_masks_by_run.at(pseudo_run));
  }

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
  auto key = PrepareEvent(runnum); // we keep the API; internally this is very light now

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

// ==========================================================================
// Small, explicit per-event prep (kept for API compatibility)
// ==========================================================================
concurrent_key_t RGAFiducialFilter::PrepareEvent(int runnum) const
{
  // We no longer build/change anything per-run, but we still update strictness.
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
  (void)runnum;
  std::lock_guard<std::mutex> lock(m_mutex);

  o_runnum->Save(runnum, key);
  const int strict = std::clamp(u_strictness_user.value_or(1), 1, 3);
  o_cal_strictness->Save(strict, key);

  if (dbg_on) {
    m_log->Info("[RGAFID][Reload] run={} key={} strictness={}", runnum, (uint64_t)key, strict);
  }
}

// ==========================================================================
// Public setter
// ==========================================================================
void RGAFiducialFilter::SetStrictness(int strictness)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  u_strictness_user = std::clamp(strictness, 1, 3);
}

// ==========================================================================
// Core filter: the cut logic lives in tiny local helpers
// ==========================================================================
bool RGAFiducialFilter::Filter(int track_index,
                               const hipo::bank* calBank,
                               const hipo::bank* ftBank,
                               concurrent_key_t key) const
{
  // ------------------ Calorimeter ------------------
  if (calBank != nullptr) {
    CalLayers h = CollectCalHitsForTrack(*calBank, track_index);

    if (h.has_any) {
      const int strictness = GetCalStrictness(key);

      // Edge cut (strictness-dependent, *very* simple)
      if (!PassCalStrictness(h, strictness)) {
        if (dbg_on && g_dbg_events_seen.load() < dbg_events) {
          m_log->Info("[RGAFID][CAL] track={} strictness={} -> edge FAIL (lv1={}, lw1={})",
                      track_index, strictness, h.lv1, h.lw1);
        }
        return false;
      }

      // Dead-PMT masks (single global map, no run gates)
      if (strictness >= 2) {
        if (!PassCalDeadPMTMasks(h, key)) {
          if (dbg_on && g_dbg_events_seen.load() < dbg_events) {
            m_log->Info("[RGAFID][CAL] track={} -> dead-PMT mask FAIL (sec={})", track_index, h.sector);
          }
          return false;
        }
      }
    }
  }

  // ------------------ Forward Tagger ---------------
  if (!PassFTFiducial(track_index, ftBank)) {
    if (dbg_on && g_dbg_events_seen.load() < dbg_events) {
      m_log->Info("[RGAFID][FT] track={} -> FT FAIL", track_index);
    }
    return false;
  }

  return true;
}

// ==========================================================================
// Calorimeter helpers: **explicit and tiny**
// ==========================================================================
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

// Super-simple edge cut on PCAL (layer 1) only
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

// Build ONE mask-map (no per-run selection). If YAML is absent/empty, use
// a tiny baked-in list based on your examples.
RGAFiducialFilter::MaskMap RGAFiducialFilter::BuildCalMaskCache(int /*runnum*/) const
{
  MaskMap out;

  // 1) Try simple YAML
  std::vector<double> flat;
  if (!TryGetVector(*this, "cal_masks_flat", flat)) {
    // 2) Fallback: baked-in defaults (sector, layer, axis, a, b)
    //    layer: 1=PCAL, 4=ECIN, 7=ECOUT; axis: 0=lv, 1=lw, 2=lu
    const double DEF[] = {
      1,1,1,  72.0,  94.5,
      1,1,1, 220.5, 234.0,
      1,4,0,  67.5,  94.5,
      1,7,0,   0.0,  40.5,
      2,1,0,  99.0, 117.0,
      2,1,0,  31.5,  49.5, // (added from second list)
      3,1,1, 346.5, 378.0,
      4,1,0, 229.5, 243.0,
      4,1,1,   0.0,  13.5,
      5,4,0,   0.0,  23.5,
      5,7,2, 193.5, 216.0,
      6,1,1, 166.5, 193.5
    };
    flat.assign(DEF, DEF + std::size(DEF));
  }

  // 3) Fill map
  auto push_axis = [](AxisWins& ax, int axis_code, float a, float b) {
    const auto lo = std::min(a, b), hi = std::max(a, b);
    if      (axis_code == AX_LV) ax.lv.emplace_back(lo, hi);
    else if (axis_code == AX_LW) ax.lw.emplace_back(lo, hi);
    else if (axis_code == AX_LU) ax.lu.emplace_back(lo, hi);
  };

  for (size_t i = 0; i + 4 < flat.size(); i += 5) {
    const int   sec  = static_cast<int>(flat[i+0]);
    const int   lay  = static_cast<int>(flat[i+1]); // 1,4,7
    const int   ax   = static_cast<int>(flat[i+2]); // 0,1,2
    const float a    = static_cast<float>(flat[i+3]);
    const float b    = static_cast<float>(flat[i+4]);

    SectorMasks& sm = out[sec];
    if      (lay == 1) push_axis(sm.pcal,  ax, a, b);
    else if (lay == 4) push_axis(sm.ecin,  ax, a, b);
    else if (lay == 7) push_axis(sm.ecout, ax, a, b);
  }

  return out;
}

// True if track **passes** masks (i.e., is NOT inside any masked window)
bool RGAFiducialFilter::PassCalDeadPMTMasks(const CalLayers& h, concurrent_key_t /*key*/) const
{
  // We store a single mask map under run=-1.
  std::lock_guard<std::mutex> lock(m_mutex);
  auto it_run = m_masks_by_run.find(-1);
  if (it_run == m_masks_by_run.end()) return true; // nothing loaded

  const auto& m = it_run->second;
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

// ==========================================================================
// FT: **explicit and tiny**
// ==========================================================================
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

    if (r < u_ft_params.rmin || r > u_ft_params.rmax) return false;

    for (auto const& h : u_ft_params.holes) {
      const double dx = x - h[1], dy = y - h[2];
      const double d  = std::sqrt(dx*dx + dy*dy);
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

// ==========================================================================
// Accessors (kept for API compatibility)
// ==========================================================================
int RGAFiducialFilter::GetRunNum(concurrent_key_t key) const { return o_runnum->Load(key); }
int RGAFiducialFilter::GetCalStrictness(concurrent_key_t key) const { return o_cal_strictness->Load(key); }

// ==========================================================================
// Debug dumps
// ==========================================================================
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
    (void)sec; (void)runnum;
    sumv(sm.pcal.lv); sumv(sm.pcal.lw); sumv(sm.pcal.lu);
    sumv(sm.ecin.lv); sumv(sm.ecin.lw); sumv(sm.ecin.lu);
    sumv(sm.ecout.lv);sumv(sm.ecout.lw);sumv(sm.ecout.lu);
  }
  m_log->Info("[RGAFID][MASK] run={} sectors={} total_windows={}", runnum, mm.size(), total);
}

} // namespace iguana::clas12