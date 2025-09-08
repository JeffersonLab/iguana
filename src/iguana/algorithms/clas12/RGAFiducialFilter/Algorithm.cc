#include "Algorithm.h"  // our header in the same folder

#include "iguana/hipo/BankUtils.h"   // GetBank / GetBankIndex helpers
#include "iguana/services/ConcurrentParam.h"

#include <yaml-cpp/yaml.h>

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <limits>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

// -------------------------------------------------------------------------------------------------
// Implementation notes
//  * YAML is REQUIRED. We error out if the file is missing or if a required key is absent.
//  * FT params are loaded from YAML into u_ft_params (no compiled-in defaults used anymore).
//  * Cal strictness: default comes from YAML; a programmatic SetStrictness() still overrides it.
//  * CVT wedges/layers are also loaded from YAML (edge_layers, edge_min, phi_forbidden_deg).
//  * Run() calls Filter(...) for each track. We only compute the decision here.
// -------------------------------------------------------------------------------------------------

namespace iguana::clas12 {

// ---------- small local helpers ----------

static bool banklist_has(hipo::banklist& banks, const char* name) {
  for (auto& b : banks) if (b.getSchema().getName() == name) return true;
  return false;
}

// Build path to this algorithm's YAML (installed under IGUANA_ETCDIR).
static inline std::string GetAlgConfigPath() {
  std::ostringstream os;
  os << IGUANA_ETCDIR << "/clas12/RGAFiducialFilter/Config.yaml";
  return os.str();
}

// ---------- optional env toggles (debug) ----------

bool RGAFiducialFilter::EnvOn(const char* name) {
  if (const char* v = std::getenv(name)) {
    return std::string(v) == "1" || std::string(v) == "true";
  }
  return false;
}
int RGAFiducialFilter::EnvInt(const char* name, int def) {
  if (const char* v = std::getenv(name)) {
    try { return std::stoi(v); } catch (...) { return def; }
  }
  return def;
}

// ---------- CVT params kept in this TU (no header changes needed) ----------

namespace {
struct CVTParams {
  std::vector<int>    edge_layers;        // e.g. {1,3,5,7,12}
  double              edge_min = 0.0;     // > edge_min
  std::vector<double> phi_forbidden_deg;  // flattened pairs (open intervals)
};
CVTParams g_cvt;
int g_yaml_cal_strictness = 1;
} // namespace

// -------------------------------------------------------------------------------------------------
// YAML LOADING (REQUIRED)
// -------------------------------------------------------------------------------------------------
void RGAFiducialFilter::LoadConfigFromYAML() {
  const std::string cfg_path = GetAlgConfigPath();

  YAML::Node root;
  try {
    root = YAML::LoadFile(cfg_path);
  } catch (const std::exception& e) {
    std::ostringstream msg;
    msg << "[RGAFID] Required Config.yaml not found or unreadable at: "
        << cfg_path << " (" << e.what() << ")";
    if (m_log) m_log->Error("{}", msg.str());
    throw std::runtime_error(msg.str());
  }

  auto top = root["clas12::RGAFiducialFilter"];
  if (!top) {
    std::ostringstream msg;
    msg << "[RGAFID] Missing top-level key 'clas12::RGAFiducialFilter' in "
        << cfg_path;
    if (m_log) m_log->Error("{}", msg.str());
    throw std::runtime_error(msg.str());
  }

  // ---- calorimeter.strictness (REQUIRED default unless overridden by SetStrictness)
  {
    auto cal = top["calorimeter"];
    if (!cal || !cal["strictness"] || cal["strictness"].size() < 1) {
      std::ostringstream msg;
      msg << "[RGAFID] Missing required 'calorimeter.strictness' in " << cfg_path;
      if (m_log) m_log->Error("{}", msg.str());
      throw std::runtime_error(msg.str());
    }
    g_yaml_cal_strictness = cal["strictness"][0].as<int>();
    if (g_yaml_cal_strictness < 1 || g_yaml_cal_strictness > 3) {
      std::ostringstream msg;
      msg << "[RGAFID] 'calorimeter.strictness' must be 1, 2, or 3 (got "
          << g_yaml_cal_strictness << ")";
      if (m_log) m_log->Error("{}", msg.str());
      throw std::runtime_error(msg.str());
    }
  }

  // ---- forward_tagger (REQUIRED)
  {
    auto ft = top["forward_tagger"];
    if (!ft) {
      std::ostringstream msg;
      msg << "[RGAFID] Missing required block 'forward_tagger' in " << cfg_path;
      if (m_log) m_log->Error("{}", msg.str());
      throw std::runtime_error(msg.str());
    }

    if (!ft["radius"] || ft["radius"].size() != 2) {
      std::ostringstream msg;
      msg << "[RGAFID] 'forward_tagger.radius' must be a 2-element list [rmin, rmax]";
      if (m_log) m_log->Error("{}", msg.str());
      throw std::runtime_error(msg.str());
    }
    float rmin = ft["radius"][0].as<float>();
    float rmax = ft["radius"][1].as<float>();
    if (!(std::isfinite(rmin) && std::isfinite(rmax)) || !(rmin > 0.f && rmax > rmin)) {
      std::ostringstream msg;
      msg << "[RGAFID] Invalid 'forward_tagger.radius': rmin=" << rmin
          << ", rmax=" << rmax << " (require 0 < rmin < rmax)";
      if (m_log) m_log->Error("{}", msg.str());
      throw std::runtime_error(msg.str());
    }

    auto hf = ft["holes_flat"];
    if (!hf || hf.size() == 0 || (hf.size() % 3) != 0) {
      std::ostringstream msg;
      msg << "[RGAFID] 'forward_tagger.holes_flat' must be non-empty with length "
             "multiple of 3: [R1,cx1,cy1, R2,cx2,cy2, ...]";
      if (m_log) m_log->Error("{}", msg.str());
      throw std::runtime_error(msg.str());
    }

    // commit to the instance
    u_ft_params.rmin = rmin;
    u_ft_params.rmax = rmax;
    u_ft_params.holes.clear();
    u_ft_params.holes.reserve(hf.size()/3);
    for (std::size_t i = 0; i < hf.size(); i += 3) {
      float R  = hf[i+0].as<float>();
      float cx = hf[i+1].as<float>();
      float cy = hf[i+2].as<float>();
      if (!(std::isfinite(R) && std::isfinite(cx) && std::isfinite(cy)) || R <= 0.f) {
        std::ostringstream msg;
        msg << "[RGAFID] Invalid FT hole triple at index " << (i/3)
            << " -> (R=" << R << ", cx=" << cx << ", cy=" << cy << ")";
        if (m_log) m_log->Error("{}", msg.str());
        throw std::runtime_error(msg.str());
      }
      u_ft_params.holes.push_back({R, cx, cy});
    }
  }

  // ---- cvt (REQUIRED)
  {
    auto cvt = top["cvt"];
    if (!cvt) {
      std::ostringstream msg;
      msg << "[RGAFID] Missing required block 'cvt' in " << cfg_path;
      if (m_log) m_log->Error("{}", msg.str());
      throw std::runtime_error(msg.str());
    }

    if (!cvt["edge_layers"] || cvt["edge_layers"].size() == 0) {
      std::ostringstream msg;
      msg << "[RGAFID] 'cvt.edge_layers' must be a non-empty list";
      if (m_log) m_log->Error("{}", msg.str());
      throw std::runtime_error(msg.str());
    }
    g_cvt.edge_layers.clear();
    for (auto v : cvt["edge_layers"]) g_cvt.edge_layers.push_back(v.as<int>());

    if (!cvt["edge_min"] || cvt["edge_min"].size() < 1) {
      std::ostringstream msg;
      msg << "[RGAFID] 'cvt.edge_min' must be provided";
      if (m_log) m_log->Error("{}", msg.str());
      throw std::runtime_error(msg.str());
    }
    g_cvt.edge_min = cvt["edge_min"][0].as<double>();

    if (!cvt["phi_forbidden_deg"] || (cvt["phi_forbidden_deg"].size() % 2) != 0) {
      std::ostringstream msg;
      msg << "[RGAFID] 'cvt.phi_forbidden_deg' must have an even number of values "
             "(pairs of (lo, hi) open intervals)";
      if (m_log) m_log->Error("{}", msg.str());
      throw std::runtime_error(msg.str());
    }
    g_cvt.phi_forbidden_deg.clear();
    for (auto v : cvt["phi_forbidden_deg"]) g_cvt.phi_forbidden_deg.push_back(v.as<double>());
  }

  DumpFTParams();
}

void RGAFiducialFilter::DumpFTParams() const {
  if (!m_log) return;
  std::ostringstream os;
  os << "[RGAFID] FT: rmin=" << u_ft_params.rmin << ", rmax=" << u_ft_params.rmax
     << ", holes=" << u_ft_params.holes.size();
  m_log->Info("{}", os.str());
}

// -------------------------------------------------------------------------------------------------
// USER OVERRIDE
// -------------------------------------------------------------------------------------------------
void RGAFiducialFilter::SetStrictness(int strictness) {
  u_strictness_user = strictness;
}

// -------------------------------------------------------------------------------------------------
// LIFECYCLE
// -------------------------------------------------------------------------------------------------
void RGAFiducialFilter::Start(hipo::banklist& banks)
{
  // Bank presence
  b_particle = GetBankIndex(banks, "REC::Particle");
  if (banklist_has(banks, "RUN::config"))        b_config = GetBankIndex(banks, "RUN::config");
  if (banklist_has(banks, "REC::Calorimeter")) { b_calor  = GetBankIndex(banks, "REC::Calorimeter"); m_have_calor = true; }
  if (banklist_has(banks, "REC::ForwardTagger")){ b_ft    = GetBankIndex(banks, "REC::ForwardTagger"); m_have_ft = true; }
  if (banklist_has(banks, "REC::Traj"))         { b_traj  = GetBankIndex(banks, "REC::Traj"); m_have_traj = true; }

  // Debug toggles (optional)
  dbg_on     = EnvOn("IGUANA_RGAFID_DBG");
  dbg_ft     = EnvOn("IGUANA_RGAFID_DBG_FT");
  dbg_events = EnvInt("IGUANA_RGAFID_N", 0);

  // Require YAML and load all params
  LoadConfigFromYAML();

  // Initialize concurrent params if you use them elsewhere
  if (!o_runnum)         o_runnum         = std::make_unique<ConcurrentParam<int>>(0);
  if (!o_cal_strictness) o_cal_strictness = std::make_unique<ConcurrentParam<int>>(u_strictness_user ? *u_strictness_user
                                                                                                     : g_yaml_cal_strictness);
}

void RGAFiducialFilter::Run(hipo::banklist& banks) const
{
  auto& particle = GetBank(banks, b_particle, "REC::Particle");
  auto* cal  = m_have_calor ? &GetBank(banks, b_calor, "REC::Calorimeter") : nullptr;
  auto* ft   = m_have_ft    ? &GetBank(banks, b_ft,    "REC::ForwardTagger") : nullptr;
  auto* traj = m_have_traj  ? &GetBank(banks, b_traj,  "REC::Traj") : nullptr;
  auto& conf = GetBank(banks, b_config, "RUN::config");

  const int ntrk = particle.getRows();

  for (int i = 0; i < ntrk; ++i) {
    const bool pass = Filter(i, particle, conf, cal, ft, traj, /*key*/0);
    (void)pass; // decide-only; hook removal if desired
  }
}

// -------------------------------------------------------------------------------------------------
// CORE FILTER HELPERS
// -------------------------------------------------------------------------------------------------

RGAFiducialFilter::CalLayers
RGAFiducialFilter::CollectCalHitsForTrack(const hipo::bank& cal, int pindex)
{
  CalLayers out;
  const int n = cal.getRows();
  for (int i=0; i<n; ++i) {
    if (cal.getInt("pindex", i) != pindex) continue;
    if (cal.getInt("layer",  i) != 1     ) continue; // PCAL only
    CalHit h;
    h.sector = cal.getInt ("sector", i);
    h.lv     = cal.getFloat("lv", i);
    h.lw     = cal.getFloat("lw", i);
    h.lu     = cal.getFloat("lu", i);
    out.L1.push_back(h);
    out.has_any = true;
  }
  return out;
}

bool RGAFiducialFilter::PassCalStrictness(const CalLayers& H, int strictness)
{
  if (!H.has_any) return true; // no PCAL -> pass

  float min_lv = std::numeric_limits<float>::infinity();
  float min_lw = std::numeric_limits<float>::infinity();
  for (auto const& hit : H.L1) {
    if (std::isfinite(hit.lv) && hit.lv < min_lv) min_lv = hit.lv;
    if (std::isfinite(hit.lw) && hit.lw < min_lw) min_lw = hit.lw;
  }

  const float thr = (strictness==1 ? 9.0f : strictness==2 ? 13.5f : 18.0f);
  return !(min_lv < thr || min_lw < thr);
}

bool RGAFiducialFilter::PassFTFiducial(int pindex, const hipo::bank* ftBank) const
{
  if (!ftBank) return true;

  const auto& ft = *ftBank;
  const int n = ft.getRows();
  for (int i=0; i<n; ++i) {
    if (ft.getInt("pindex", i) != pindex) continue;

    const double x = ft.getFloat("x", i);
    const double y = ft.getFloat("y", i);
    const double r = std::hypot(x, y);

    if (r < u_ft_params.rmin) return false;
    if (r > u_ft_params.rmax) return false;

    for (auto const& H : u_ft_params.holes) {
      const double R = H[0], cx = H[1], cy = H[2];
      const double d = std::hypot(x-cx, y-cy);
      if (d < R) return false;
    }
    return true; // first association decides
  }
  return true; // no FT association -> pass
}

bool RGAFiducialFilter::PassCVTFiducial(int pindex, const hipo::bank* trajBank, int /*strictness*/) const
{
  if (!trajBank) return true;

  const auto& traj = *trajBank;
  const int n = traj.getRows();

  std::map<int, double> edge_at_layer;
  double x12 = 0.0, y12 = 0.0; bool saw12 = false;

  for (int i=0; i<n; ++i) {
    if (traj.getInt("pindex", i)   != pindex) continue;
    if (traj.getInt("detector", i) != 5     ) continue; // CVT

    const int layer = traj.getInt("layer", i);
    const double e  = traj.getFloat("edge", i);

    if (std::find(g_cvt.edge_layers.begin(), g_cvt.edge_layers.end(), layer) != g_cvt.edge_layers.end()) {
      edge_at_layer[layer] = e;
    }
    if (layer == 12) {
      x12 = traj.getFloat("x", i);
      y12 = traj.getFloat("y", i);
      saw12 = true;
    }
  }

  for (int L : g_cvt.edge_layers) {
    auto it = edge_at_layer.find(L);
    if (it == edge_at_layer.end()) continue; // missing layer -> pass
    if (!(it->second > g_cvt.edge_min)) return false;
  }

  if (saw12 && !g_cvt.phi_forbidden_deg.empty()) {
    double phi = std::atan2(y12, x12) * (180.0/M_PI);
    if (phi < 0) phi += 360.0;
    for (std::size_t i=0; i+1<g_cvt.phi_forbidden_deg.size(); i+=2) {
      const double lo = g_cvt.phi_forbidden_deg[i];
      const double hi = g_cvt.phi_forbidden_deg[i+1];
      if (phi > lo && phi < hi) return false;
    }
  }

  return true;
}

bool RGAFiducialFilter::PassDCFiducial(int pindex,
                                       const hipo::bank& particleBank,
                                       const hipo::bank& configBank,
                                       const hipo::bank* trajBank) const
{
  if (!trajBank) return true;

  const int pid = particleBank.getInt("pid", pindex);
  const bool isNeg = (pid== 11 || pid==-211 || pid==-321 || pid==-2212);
  const bool isPos = (pid==-11 || pid== 211 || pid== 321 || pid== 2212);
  if (!(isNeg || isPos)) return true;

  const float torus = configBank.getFloat("torus", 0);
  const bool electron_out = (torus == 1.0f);
  const bool particle_inb = (electron_out ? isPos : isNeg);
  const bool particle_out = !particle_inb;

  const double px = particleBank.getFloat("px", pindex);
  const double py = particleBank.getFloat("py", pindex);
  const double pz = particleBank.getFloat("pz", pindex);
  const double rho = std::hypot(px, py);
  const double theta = std::atan2(rho, (pz==0.0 ? 1e-9 : pz)) * (180.0/M_PI);

  double e1=0.0, e2=0.0, e3=0.0;
  const auto& traj = *trajBank;
  const int n = traj.getRows();
  for (int i=0; i<n; ++i) {
    if (traj.getInt("pindex", i) != pindex) continue;
    if (traj.getInt("detector", i) != 6)    continue; // DC
    const int layer = traj.getInt("layer", i);
    const double e  = traj.getFloat("edge", i);
    if      (layer== 6) e1 = e;
    else if (layer==18) e2 = e;
    else if (layer==36) e3 = e;
  }

  if (particle_inb) {
    if (theta < 10.0) return (e1>10.0 && e2>10.0 && e3>10.0);
    return (e1>3.0 && e2>3.0 && e3>10.0);
  } else if (particle_out) {
    return (e1>3.0 && e2>3.0 && e3>10.0);
  }
  return true;
}

bool RGAFiducialFilter::Filter(int track_index,
                               const hipo::bank& particleBank,
                               const hipo::bank& configBank,
                               const hipo::bank* calBank,
                               const hipo::bank* ftBank,
                               const hipo::bank* trajBank,
                               concurrent_key_t /*key*/) const
{
  const int pid = particleBank.getInt("pid", track_index);

  const int strictness = u_strictness_user.value_or(g_yaml_cal_strictness);

  bool pass = true;

  if (pid == 11 || pid == 22) {
    if (calBank) {
      auto calhits = CollectCalHitsForTrack(*calBank, track_index);
      pass = pass && PassCalStrictness(calhits, strictness);
    }
    pass = pass && PassFTFiducial(track_index, ftBank);
  }

  if (pid== 211 || pid== 321 || pid== 2212 ||
      pid==-211 || pid==-321 || pid==-2212) {
    pass = pass && PassCVTFiducial(track_index, trajBank, strictness);
    pass = pass && PassDCFiducial(track_index, particleBank, configBank, trajBank);
  }

  return pass;
}

} // namespace iguana::clas12