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

  // --- forward_tagger.holes_flat ---
  try {
    auto flat = GetOptionVector<double>("forward_tagger.holes_flat",
                                        {"forward_tagger", "holes_flat"});
    if (!flat.empty()) {
      u_ft_params.holes.clear();
      for (size_t i = 0; i + 2 < flat.size(); i += 3) {
        u_ft_params.holes.push_back({
          static_cast<float>(flat[i]),
          static_cast<float>(flat[i+1]),
          static_cast<float>(flat[i+2])
        });
      }
      m_log->Info("[RGAFID][CFG] YAML FT holes set: {} holes", u_ft_params.holes.size());
    }
  } catch (...) {
    auto flat = getDoubles("clas12::RGAFiducialFilter.forward_tagger.holes_flat",
                           "clas12::RGAFiducialFilter.forward_tagger.holes_flat");
    if (!flat.empty()) {
      u_ft_params.holes.clear();
      for (size_t i = 0; i + 2 < flat.size(); i += 3) {
        u_ft_params.holes.push_back({
          static_cast<float>(flat[i]),
          static_cast<float>(flat[i+1]),
          static_cast<float>(flat[i+2])
        });
      }
      m_log->Info("[RGAFID][CFG] YAML FT holes set (qualified): {} holes", u_ft_params.holes.size());
    } else {
      m_log->Info("[RGAFID][CFG] FT holes not in YAML; defaults kept.");
    }
  }

  // --- cvt.edge_layers ---
  try {
    auto v = GetOptionVector<int>("cvt.edge_layers", {"cvt", "edge_layers"});
    if (!v.empty()) {
      u_cvt_params.edge_layers.assign(v.begin(), v.end());
      m_log->Info("[RGAFID][CFG] YAML CVT edge_layers = {} entries", u_cvt_params.edge_layers.size());
    }
  } catch (...) {
    auto v = getInts("clas12::RGAFiducialFilter.cvt.edge_layers",
                     "clas12::RGAFiducialFilter.cvt.edge_layers");
    if (!v.empty()) {
      u_cvt_params.edge_layers.assign(v.begin(), v.end());
      m_log->Info("[RGAFID][CFG] YAML CVT edge_layers (qualified) = {} entries",
                  u_cvt_params.edge_layers.size());
    }
  }

  // --- cvt.edge_min ---
  try {
    auto v = GetOptionVector<double>("cvt.edge_min", {"cvt", "edge_min"});
    if (!v.empty()) {
      u_cvt_params.edge_min = static_cast<float>(v.front());
      m_log->Info("[RGAFID][CFG] YAML CVT edge_min = {:.3f}", u_cvt_params.edge_min);
    }
  } catch (...) {
    auto v = getDoubles("clas12::RGAFiducialFilter.cvt.edge_min",
                        "clas12::RGAFiducialFilter.cvt.edge_min");
    if (!v.empty()) {
      u_cvt_params.edge_min = static_cast<float>(v.front());
      m_log->Info("[RGAFID][CFG] YAML CVT edge_min (qualified) = {:.3f}", u_cvt_params.edge_min);
    }
  }

  // --- cvt.phi_forbidden_deg ---
  try {
    auto v = GetOptionVector<double>("cvt.phi_forbidden_deg", {"cvt", "phi_forbidden_deg"});
    if (!v.empty()) {
      u_cvt_params.phi_forbidden_deg.clear();
      for (double d : v) u_cvt_params.phi_forbidden_deg.push_back(static_cast<float>(d));
      m_log->Info("[RGAFID][CFG] YAML CVT phi_forbidden_deg = {} values",
                  u_cvt_params.phi_forbidden_deg.size());
    }
  } catch (...) {
    auto v = getDoubles("clas12::RGAFiducialFilter.cvt.phi_forbidden_deg",
                        "clas12::RGAFiducialFilter.cvt.phi_forbidden_deg");
    if (!v.empty()) {
      u_cvt_params.phi_forbidden_deg.clear();
      for (double d : v) u_cvt_params.phi_forbidden_deg.push_back(static_cast<float>(d));
      m_log->Info("[RGAFID][CFG] YAML CVT phi_forbidden_deg (qualified) = {} values",
                  u_cvt_params.phi_forbidden_deg.size());
    }
  }
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
  if (dbg_on || dbg_ft) {
    DumpFTParams();
    DumpCVTParams();
  }

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
  if (banklist_has(banks, "REC::Traj")) {
    b_traj = GetBankIndex(banks, "REC::Traj");
    m_have_traj = true;
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

  particleBank.getMutableRowList().filter([this, calBankPtr, ftBankPtr, trajBankPtr, key](auto, auto row) {
    const int track_index = row;
    const bool accept = Filter(track_index, calBankPtr, ftBankPtr, trajBankPtr, key);

    if (dbg_on && dbg_events > 0) {
      int seen = ++g_dbg_events_seen;
      if (seen <= dbg_events) {
        m_log->Info("[RGAFID][track={}] -> {}", track_index, accept ? "ACCEPT" : "REJECT");
      }
    }
    return accept ? 1 : 0;
  });
}

// -------- filter core (PCAL) ----------
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
  if (h.L1.empty()) return true; // no PCAL association -> no PCAL cut

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

// -------- filter core (FT) ----------
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

// -------- filter core (CVT) ----------
bool RGAFiducialFilter::PassCVTFiducial(int track_index, const hipo::bank* trajBank, int strictness) const
{
  (void)strictness; // currently unused for CVT; edges and phi wedges always applied
  if (trajBank == nullptr) return true;

  // CVT rows are in REC::Traj with detector == 5
  // We need edges for layers in u_cvt_params.edge_layers
  // and x,y,z at layer 12 for phi computation
  const int nrows = trajBank->getRows();

  // initialize edges as pass by default (mimic Java)
  std::vector<float> edges(13, 1.0f); // layers 0..12, we use 1..12
  bool have_l12 = false;
  double x12 = 0.0, y12 = 0.0, z12 = 0.0;

  for (int i = 0; i < nrows; ++i) {
    if (trajBank->getInt("pindex", i) != track_index) continue;
    if (trajBank->getInt("detector", i) != 5) continue; // 5 = CVT

    const int layer = trajBank->getInt("layer", i);
    if (layer >= 0 && layer < static_cast<int>(edges.size())) {
      if (trajBank->hasFloat("edge"))
        edges[layer] = trajBank->getFloat("edge", i);
    }

    if (layer == 12) {
      if (trajBank->hasFloat("x")) x12 = trajBank->getFloat("x", i);
      if (trajBank->hasFloat("y")) y12 = trajBank->getFloat("y", i);
      if (trajBank->hasFloat("z")) z12 = trajBank->getFloat("z", i);
      have_l12 = true;
    }
  }

  // edge test on configured layers (missing layers remain 1.0 and pass)
  bool edge_test = true;
  for (int L : u_cvt_params.edge_layers) {
    if (L >= 0 && L < static_cast<int>(edges.size())) {
      if (!(edges[L] > u_cvt_params.edge_min)) { edge_test = false; break; }
    }
  }
  if (!edge_test) return false;

  // always apply forbidden phi wedges if we have layer 12 position
  if (have_l12 && !u_cvt_params.phi_forbidden_deg.empty()) {
    double phi = std::atan2(y12, x12) * 180.0 / M_PI;
    if (phi < 0) phi += 360.0;
    const auto& v = u_cvt_params.phi_forbidden_deg;
    for (size_t i = 0; i + 1 < v.size(); i += 2) {
      const double lo = v[i], hi = v[i+1]; // open interval (lo,hi)
      if (phi > lo && phi < hi) return false;
    }
  }

  return true;
}

bool RGAFiducialFilter::Filter(int track_index,
                               const hipo::bank* calBank,
                               const hipo::bank* ftBank,
                               const hipo::bank* trajBank,
                               concurrent_key_t key) const
{
  // PCAL strictness
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

  // CVT edge and phi wedges
  if (!PassCVTFiducial(track_index, trajBank, GetCalStrictness(key))) {
    if (dbg_on && g_dbg_events_seen.load() < dbg_events) {
      m_log->Info("[RGAFID][CVT] track={} -> CVT FAIL", track_index);
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

void RGAFiducialFilter::DumpCVTParams() const {
  m_log->Info("[RGAFID][CVT] edge_min={:.3f} edge_layers={}", u_cvt_params.edge_min,
              u_cvt_params.edge_layers.size());
  if (!u_cvt_params.edge_layers.empty()) {
    std::string s = "   layers:";
    for (int L : u_cvt_params.edge_layers) { s += " " + std::to_string(L); }
    m_log->Info("{}", s);
  }
  if (!u_cvt_params.phi_forbidden_deg.empty()) {
    std::string s = "   phi wedges (deg):";
    for (size_t i = 0; i + 1 < u_cvt_params.phi_forbidden_deg.size(); i += 2) {
      s += " (" + std::to_string((int)u_cvt_params.phi_forbidden_deg[i])
        + "," + std::to_string((int)u_cvt_params.phi_forbidden_deg[i+1]) + ")";
    }
    m_log->Info("{}", s);
  }
}

} // namespace iguana::clas12