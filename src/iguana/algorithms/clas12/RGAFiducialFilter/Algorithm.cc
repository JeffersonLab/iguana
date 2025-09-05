#include "Algorithm.h"
#include "iguana/services/YAMLReader.h"

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdlib>
#include <limits>
#include <mutex>
#include <optional>

namespace {
std::atomic<int> g_dbg_events_seen{0};
}

namespace iguana::clas12 {

REGISTER_IGUANA_ALGORITHM(RGAFiducialFilter);

// -------- small utils ----------
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
bool RGAFiducialFilter::banklist_has(hipo::banklist& banks, const char* name) {
  for (auto& b : banks) if (b.getSchema().getName() == name) return true;
  return false;
}

// -------- YAML (algo-local Config.yaml) ----------
void RGAFiducialFilter::LoadConfigFromYAML()
{
  // Make the framework search THIS algorithm's directory first (same as ZVertexFilter)
  ParseYAMLConfig();

  // If there is no config at all, keep defaults and log once
  if (!GetConfig()) {
    m_log->Info("[RGAFID][CFG] No Config.yaml found for RGAFiducialFilter; using built-in defaults.");
    return;
  }

  // Helper to try two forms: bare + fully-qualified (defensive)
  auto getDoubles = [&](const char* bare, const char* qualified) -> std::vector<double> {
    try { return GetOptionVector<double>(bare, {bare}); } catch (...) {}
    try { return GetOptionVector<double>(qualified, {qualified}); } catch (...) {}
    return {};
  };
  auto getInts = [&](const char* bare, const char* qualified) -> std::vector<int> {
    try { return GetOptionVector<int>(bare, {bare}); } catch (...) {}
    try { return GetOptionVector<int>(qualified, {qualified}); } catch (...) {}
    return {};
  };

  // --- calorimeter.strictness ---
  try {
    auto v = GetOptionVector<int>("calorimeter.strictness",
                                  {"calorimeter", "strictness"});
    if (!v.empty()) {
      u_strictness_user = std::clamp(v.front(), 1, 3);
      m_log->Info("[RGAFID][CFG] YAML strictness = {}", *u_strictness_user);
    }
  } catch (...) {
    auto v = getInts("clas12::RGAFiducialFilter.calorimeter.strictness",
                     "clas12::RGAFiducialFilter.calorimeter.strictness");
    if (!v.empty()) {
      u_strictness_user = std::clamp(v.front(), 1, 3);
      m_log->Info("[RGAFID][CFG] YAML strictness (qualified) = {}", *u_strictness_user);
    } else {
      m_log->Info("[RGAFID][CFG] strictness not in YAML; default will be used.");
    }
  }

  // --- forward_tagger.radius ---
  try {
    auto r = GetOptionVector<double>("forward_tagger.radius",
                                     {"forward_tagger", "radius"});
    if (r.size() >= 2) {
      float a = static_cast<float>(r[0]);
      float b = static_cast<float>(r[1]);
      u_ft_params.rmin = std::min(a, b);
      u_ft_params.rmax = std::max(a, b);
      m_log->Info("[RGAFID][CFG] YAML FT radius = [{:.3f}, {:.3f}]",
                  u_ft_params.rmin, u_ft_params.rmax);
    }
  } catch (...) {
    auto r = getDoubles("clas12::RGAFiducialFilter.forward_tagger.radius",
                        "clas12::RGAFiducialFilter.forward_tagger.radius");
    if (r.size() >= 2) {
      float a = static_cast<float>(r[0]);
      float b = static_cast<float>(r[1]);
      u_ft_params.rmin = std::min(a, b);
      u_ft_params.rmax = std::max(a, b);
      m_log->Info("[RGAFID][CFG] YAML FT radius (qualified) = [{:.3f}, {:.3f}]",
                  u_ft_params.rmin, u_ft_params.rmax);
    } else {
      m_log->Info("[RGAFID][CFG] FT radius not in YAML; defaults kept.");
    }
  }

  // NOTE: We no longer attempt to read forward_tagger.holes_flat from YAML.
  //       The compiled-in defaults are used, which also match the validator overlay.
}

void RGAFiducialFilter::SetStrictness(int s) {
  u_strictness_user = std::clamp(s, 1, 3);
}

// -------- lifecycle ----------
void RGAFiducialFilter::Start(hipo::banklist& banks)
{
  // Debug knobs
  dbg_on     = EnvOn("IGUANA_RGAFID_DEBUG");
  dbg_ft     = EnvOn("IGUANA_RGAFID_DEBUG_FT");
  dbg_events = EnvInt("IGUANA_RGAFID_DEBUG_EVENTS", 0);

  // Load algo-local YAML (safe if missing)
  LoadConfigFromYAML();

  // concurrent params
  o_runnum         = ConcurrentParamFactory::Create<int>();
  o_cal_strictness = ConcurrentParamFactory::Create<int>();

  // If strictness not set programmatically or via YAML, default to 1
  if (!u_strictness_user.has_value()) u_strictness_user = 1;

  if (dbg_on) {
    m_log->Info("[RGAFID][DEBUG] ft={} events={}", dbg_ft, dbg_events);
    m_log->Info("[RGAFID] strictness = {}", *u_strictness_user);
  }
  if (dbg_on || dbg_ft) DumpFTParams();

  // required banks
  b_particle = GetBankIndex(banks, "REC::Particle");
  b_config   = GetBankIndex(banks, "RUN::config");

  // optional banks
  if (banklist_has(banks, "REC::Calorimeter")) {
    b_calor = GetBankIndex(banks, "REC::Calorimeter");
    m_have_calor = true;
  }
  if (banklist_has(banks, "REC::ForwardTagger")) {
    b_ft = GetBankIndex(banks, "REC::ForwardTagger");
    m_have_ft = true;
  }
  // CVT / DC trajectories come from REC::Traj (detector==5 or 6)
  if (banklist_has(banks, "REC::Traj")) {
    b_traj = GetBankIndex(banks, "REC::Traj");
    m_have_traj = true;
  } else {
    m_have_traj = false;
  }
}

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
  o_cal_strictness->Save(std::clamp(u_strictness_user.value_or(1), 1, 3), key);

  if (dbg_on) {
    m_log->Info("[RGAFID][Reload] run={} key={} strictness={}", runnum, (uint64_t)key, GetCalStrictness(key));
  }
}

void RGAFiducialFilter::Run(hipo::banklist& banks) const
{
  auto& particleBank = GetBank(banks, b_particle, "REC::Particle");
  auto& configBank   = GetBank(banks, b_config,   "RUN::config");

  const int runnum = configBank.getInt("run", 0);
  auto key = PrepareEvent(runnum);

  if (dbg_on) {
    static std::atomic<int> once{0};
    if (once.fetch_add(1) == 0) {
      m_log->Info("[RGAFID] Run(): run={} have_calor={} have_ft={} have_traj={} strictness={}",
                  runnum, m_have_calor, m_have_ft, m_have_traj, GetCalStrictness(key));
    }
  }

  const hipo::bank* calBankPtr  = m_have_calor ? &GetBank(banks, b_calor, "REC::Calorimeter") : nullptr;
  const hipo::bank* ftBankPtr   = m_have_ft    ? &GetBank(banks, b_ft,    "REC::ForwardTagger") : nullptr;
  const hipo::bank* trajBankPtr = m_have_traj  ? &GetBank(banks, b_traj,  "REC::Traj") : nullptr;

  particleBank.getMutableRowList().filter([this, &particleBank, &configBank, calBankPtr, ftBankPtr, trajBankPtr, key](auto, auto row) {
    const int track_index = row;
    const bool accept = Filter(track_index, particleBank, configBank, calBankPtr, ftBankPtr, trajBankPtr, key);

    if (dbg_on && dbg_events > 0) {
      int seen = ++g_dbg_events_seen;
      if (seen <= dbg_events) {
        m_log->Info("[RGAFID][track={}] -> {}", track_index, accept ? "ACCEPT" : "REJECT");
      }
    }
    return accept ? 1 : 0;
  });
}

// -------- filter core ----------
RGAFiducialFilter::CalLayers
RGAFiducialFilter::CollectCalHitsForTrack(const hipo::bank& calBank, int pindex)
{
  CalLayers out;
  const int nrows = calBank.getRows();
  for (int i = 0; i < nrows; ++i) {
    if (calBank.getInt("pindex", i) != pindex) continue;
    out.has_any = true;
    const int layer = calBank.getInt("layer", i);
    if (layer == 1) { // PCAL only
      CalHit hit;
      hit.sector = calBank.getInt("sector", i);
      hit.lv     = calBank.getFloat("lv", i);
      hit.lw     = calBank.getFloat("lw", i);
      hit.lu     = calBank.getFloat("lu", i);
      out.L1.push_back(hit);
    }
  }
  return out;
}

bool RGAFiducialFilter::PassCalStrictness(const CalLayers& h, int strictness)
{
  if (h.L1.empty()) return true; // no PCAL association => no PCAL cut

  float min_lv = std::numeric_limits<float>::infinity();
  float min_lw = std::numeric_limits<float>::infinity();
  for (const auto& hit : h.L1) {
    if (hit.lv < min_lv) min_lv = hit.lv;
    if (hit.lw < min_lw) min_lw = hit.lw;
  }

  switch (strictness) {
    case 1: return !(min_lw <  9.0f || min_lv <  9.0f);
    case 2: return !(min_lw < 13.5f || min_lv < 13.5f);
    case 3: return !(min_lw < 18.0f || min_lv < 18.0f);
    default: return false;
  }
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
      if (d < h[0]) return false;
    }

    return true; // first associated FT row decides
  }

  return true; // no FT association -> pass
}

bool RGAFiducialFilter::PassCVTFiducial(int track_index, const hipo::bank* trajBank, int /*strictness*/) const
{
  // Use REC::Traj only; require detector==5 rows for this pindex.
  if (trajBank == nullptr) return true;

  constexpr int kCVTDetectorID = 5;

  double edge_1 = 1.0, edge_3 = 1.0, edge_5 = 1.0, edge_7 = 1.0, edge_12 = 1.0;
  double x12 = 0.0, y12 = 0.0;

  const int nrows = trajBank->getRows();
  for (int i = 0; i < nrows; ++i) {
    if (trajBank->getInt("pindex", i) != track_index) continue;
    if (trajBank->getInt("detector", i) != kCVTDetectorID) continue;

    int layer = trajBank->getInt("layer", i);

    // Edge exists in REC::Traj for CVT.
    double e = trajBank->getFloat("edge", i);
    if      (layer == 1)  edge_1  = e;
    else if (layer == 3)  edge_3  = e;
    else if (layer == 5)  edge_5  = e;
    else if (layer == 7)  edge_7  = e;
    else if (layer == 12) edge_12 = e;

    if (layer == 12) {
      x12 = trajBank->getFloat("x", i);
      y12 = trajBank->getFloat("y", i);
    }
  }

  // Phi wedge veto: apply ALWAYS (all strictness levels)
  const double phi_deg = [&](){
    double p = std::atan2(y12, x12) * (180.0 / M_PI);
    if (p < 0) p += 360.0;
    return p;
  }();
  const bool phi_veto =
      (phi_deg >  25.0 && phi_deg <  40.0) ||
      (phi_deg > 143.0 && phi_deg < 158.0) ||
      (phi_deg > 265.0 && phi_deg < 280.0);

  if (phi_veto) {
    if (dbg_on && g_dbg_events_seen.load() < std::max(1, dbg_events)) {
      m_log->Info("[RGAFID][CVT] track={} -> phi wedge veto; phi12={:.2f}", track_index, phi_deg);
    }
    return false;
  }

  const bool edge_test = (edge_1 > 0.0 && edge_3 > 0.0 && edge_5 > 0.0 && edge_7 > 0.0 && edge_12 > 0.0);
  if (!edge_test && dbg_on && g_dbg_events_seen.load() < std::max(1, dbg_events)) {
    m_log->Info("[RGAFID][CVT] track={} -> edge FAIL e1={:.2f} e3={:.2f} e5={:.2f} e7={:.2f} e12={:.2f}",
                track_index, edge_1, edge_3, edge_5, edge_7, edge_12);
  }

  return edge_test;
}

// --- DC fiducial: detector==6, regions at layers 6,18,36, with polarity from torus only.
bool RGAFiducialFilter::PassDCFiducial(int track_index,
                                       const hipo::bank& particleBank,
                                       const hipo::bank& configBank,
                                       const hipo::bank* trajBank) const
{
  if (trajBank == nullptr) return true; // if no trajectories, do not apply DC cut

  const int pid = particleBank.getInt("pid", track_index);
  // Only defined for e± and {±211, ±321, ±2212}; otherwise fail (same behavior as before).
  const bool isNegPid = (pid ==  11 || pid == -211 || pid == -321 || pid == -2212);
  const bool isPosPid = (pid == -11 || pid ==  211 || pid ==  321 || pid ==  2212);
  if (!(isNegPid || isPosPid)) return false;

  // Electron bending from torus polarity only
  const float torus = configBank.getFloat("torus", 0);
  const bool electron_outbending = (torus == 1.0f);
  const bool electron_inbending  = !electron_outbending;

  // Particle bending from electron polarity + charge sign
  // electron inbending  -> negatives inbending, positives outbending
  // electron outbending -> negatives outbending, positives inbending
  const bool particle_inbending  = (electron_inbending  && isNegPid) || (electron_outbending && isPosPid);
  const bool particle_outbending = !particle_inbending;

  // theta from px,py,pz
  const double px = particleBank.getFloat("px", track_index);
  const double py = particleBank.getFloat("py", track_index);
  const double pz = particleBank.getFloat("pz", track_index);
  const double rho = std::sqrt(px*px + py*py);
  const double theta_deg = std::atan2(rho, (pz==0.0 ? 1e-9 : pz)) * (180.0/M_PI);

  // DC edges by region (defaults 0 => fail unless overwritten)
  constexpr int kDCDetectorID = 6;
  double edge_1 = 0.0; // region 1 (layer 6)
  double edge_2 = 0.0; // region 2 (layer 18)
  double edge_3 = 0.0; // region 3 (layer 36)

  const int nrows = trajBank->getRows();
  for (int i=0; i<nrows; ++i) {
    if (trajBank->getInt("detector", i) != kDCDetectorID) continue;
    if (trajBank->getInt("pindex", i)   != track_index)    continue;

    const int layer = trajBank->getInt("layer", i);
    const double e  = trajBank->getFloat("edge", i);
    if      (layer ==  6) edge_1 = e;
    else if (layer == 18) edge_2 = e;
    else if (layer == 36) edge_3 = e;
  }

  if (particle_inbending) {
    if (theta_deg < 10.0) {
      return (edge_1 > 10.0 && edge_2 > 10.0 && edge_3 > 10.0);
    } else {
      return (edge_1 >  3.0 && edge_2 >  3.0 && edge_3 > 10.0);
    }
  } else if (particle_outbending) {
    return (edge_1 > 3.0 && edge_2 > 3.0 && edge_3 > 10.0);
  }

  return false; // defensive
}

bool RGAFiducialFilter::Filter(int track_index,
                               const hipo::bank& particleBank,
                               const hipo::bank& configBank,
                               const hipo::bank* calBank,
                               const hipo::bank* ftBank,
                               const hipo::bank* trajBank,
                               concurrent_key_t key) const
{
  // PCAL strictness only
  if (calBank != nullptr) {
    CalLayers h = CollectCalHitsForTrack(*calBank, track_index);
    if (h.has_any && !PassCalStrictness(h, GetCalStrictness(key))) {
      if (dbg_on && g_dbg_events_seen.load() < dbg_events) {
        float min_lv = std::numeric_limits<float>::infinity();
        float min_lw = std::numeric_limits<float>::infinity();
        for (const auto& hit : h.L1) {
          if (hit.lv < min_lv) min_lv = hit.lv;
          if (hit.lw < min_lw) min_lw = hit.lw;
        }
        m_log->Info("[RGAFID][CAL] track={} strictness={} -> edge FAIL (min lv1={:.1f}, min lw1={:.1f})",
                    track_index, GetCalStrictness(key),
                    std::isinf(min_lv)?0.f:min_lv, std::isinf(min_lw)?0.f:min_lw);
      }
      return false;
    }
  }

  // FT annulus + holes
  if (!PassFTFiducial(track_index, ftBank)) {
    if (dbg_on && g_dbg_events_seen.load() < dbg_events) {
      m_log->Info("[RGAFID][FT] track={} -> FT FAIL", track_index);
    }
    return false;
  }

  // CVT fiducial (edges + phi wedge). Optional: if no REC::Traj, pass.
  if (!PassCVTFiducial(track_index, trajBank, GetCalStrictness(key))) {
    if (dbg_on && g_dbg_events_seen.load() < dbg_events) {
      m_log->Info("[RGAFID][CVT] track={} -> CVT FAIL", track_index);
    }
    return false;
  }

  // DC fiducial (REC::Traj detector==6); optional if REC::Traj missing.
  if (!PassDCFiducial(track_index, particleBank, configBank, trajBank)) {
    if (dbg_on && g_dbg_events_seen.load() < dbg_events) {
      m_log->Info("[RGAFID][DC] track={} -> DC FAIL", track_index);
    }
    return false;
  }

  return true;
}

// -------- diag ----------
void RGAFiducialFilter::DumpFTParams() const {
  m_log->Info("[RGAFID][FT] rmin={:.3f} rmax={:.3f} holes={}",
              u_ft_params.rmin, u_ft_params.rmax, u_ft_params.holes.size());
  for (size_t i = 0; i < u_ft_params.holes.size() && i < 8; ++i) {
    const auto& h = u_ft_params.holes[i];
    m_log->Info("   hole[{}] R={:.3f} cx={:.3f} cy={:.3f}", i, h[0], h[1], h[2]);
  }
}

} // namespace iguana::clas12