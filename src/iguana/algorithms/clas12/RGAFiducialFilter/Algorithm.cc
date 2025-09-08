#include "Algorithm.h" 

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
#include <fstream>

// Default settings defined and adjustable in Config.yaml (CAL)
//
// * Forward Tagger cut on annulus and missing holes for pid==11,22
//
// * Calorimeter cut on lv,lw>9,13.5,18 depending on SetStrictness([default = 1]) for pid==11,22; 
//    !!! TO DO: implement run-by-run data/MC matching for missing PMTs, etc.
//
// * Central Detector cut on areas between 3 sectors and require tracks inside detector (edge > 0);
//    for all charged hadrons
// 
// * Drift chamber cut on region 1 > 3, region 2 > 3, region3 > 10; inbending tracks also require
//    region 1 > 10, region 2 > 10 for theta < 10, cuts for all charged leptons and hadrons

namespace iguana::clas12 {

static bool banklist_has(hipo::banklist& banks, const char* name) {
  for (auto& b : banks) if (b.getSchema().getName() == name) return true;
  return false;
}

// Build path to this algorithm's YAML (installed under IGUANA_ETCDIR).
static inline std::string GetAlgConfigPath() {
  // 1) Prefer a Config.yaml sitting next to this source file 
  std::string here = __FILE__;                       
  auto slash = here.find_last_of("/\\");
  std::string src_dir = (slash == std::string::npos) ? "." : here.substr(0, slash);
  const std::string local = src_dir + "/Config.yaml";

  // 2) Fallback: repo-relative path (running from build tree)
  const std::string repo_rel = "src/iguana/algorithms/clas12/RGAFiducialFilter/Config.yaml";

  // 3) Fallback: install-time etc path (production)
  const char* env_etc = std::getenv("IGUANA_ETCDIR");
  const std::string base_etc = env_etc ? std::string(env_etc) : std::string(IGUANA_ETCDIR);
  const std::string install = base_etc + "/clas12/RGAFiducialFilter/Config.yaml";

  // Try candidates in order
  for (const auto& p : {local, repo_rel, install}) {
    std::ifstream f(p);
    if (f.good()) return p;
  }
  // If none exist, return the highest-priority path so the existing error message reports it.
  return local;
}

// ---------- optional env toggles (for debug) ----------

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

// ---------- CVT/DC params kept in this TU ----------

namespace {
struct CVTParams {
  std::vector<int>    edge_layers;        // e.g. {1,3,5,7,12}
  double              edge_min = 0.0;     // > edge_min
  std::vector<double> phi_forbidden_deg;  // flattened pairs (open intervals)
};
CVTParams g_cvt;
int g_yaml_cal_strictness = 1;

struct DCParams {
  // Thresholds (cm)
  double theta_small_deg   = 10.0;   // theta boundary for special inbending case
  // inbending, theta < theta_small_deg
  double in_small_e1 = 10.0, in_small_e2 = 10.0, in_small_e3 = 10.0;
  // inbending, theta >= theta_small_deg
  double in_large_e1 = 3.0,  in_large_e2 = 3.0,  in_large_e3 = 10.0;
  // outbending (any theta)
  double out_e1      = 3.0,  out_e2      = 3.0,  out_e3      = 10.0;
};
DCParams g_dc;
} // namespace


// YAML LOADING (REQUIRED)
void RGAFiducialFilter::LoadConfigFromYAML() {
  const std::string cfg_path = GetAlgConfigPath();

  YAML::Node root;
  try {
    root = YAML::LoadFile(cfg_path);
  } catch (const std::exception& e) {
    std::ostringstream msg;
    msg << "[RGAFID] Required Config.yaml not found or unreadable at: "
        << cfg_path << " (" << e.what() << ")";
    throw std::runtime_error(msg.str());
  }

  auto top = root["clas12::RGAFiducialFilter"];
  if (!top) {
    std::ostringstream msg;
    msg << "[RGAFID] Missing top-level key 'clas12::RGAFiducialFilter' in "
        << cfg_path;
    throw std::runtime_error(msg.str());
  }

  // calorimeter.strictness (default == 1 unless overridden by SetStrictness)
  {
    auto cal = top["calorimeter"];
    if (!cal || !cal["strictness"] || cal["strictness"].size() < 1) {
      std::ostringstream msg;
      msg << "[RGAFID] Missing required 'calorimeter.strictness' in " << cfg_path;
      throw std::runtime_error(msg.str());
    }
    g_yaml_cal_strictness = cal["strictness"][0].as<int>();
    if (g_yaml_cal_strictness < 1 || g_yaml_cal_strictness > 3) {
      std::ostringstream msg;
      msg << "[RGAFID] 'calorimeter.strictness' must be 1, 2, or 3 (got "
          << g_yaml_cal_strictness << ")";
      throw std::runtime_error(msg.str());
    }
  }

  // forward_tagger 
  {
    auto ft = top["forward_tagger"];
    if (!ft) {
      std::ostringstream msg;
      msg << "[RGAFID] Missing required block 'forward_tagger' in " << cfg_path;
      throw std::runtime_error(msg.str());
    }

    if (!ft["radius"] || ft["radius"].size() != 2) {
      std::ostringstream msg;
      msg << "[RGAFID] 'forward_tagger.radius' must be a 2-element list [rmin, rmax]";
      throw std::runtime_error(msg.str());
    }
    float rmin = ft["radius"][0].as<float>();
    float rmax = ft["radius"][1].as<float>();
    if (!(std::isfinite(rmin) && std::isfinite(rmax)) || !(rmin > 0.f && rmax > rmin)) {
      std::ostringstream msg;
      msg << "[RGAFID] Invalid 'forward_tagger.radius': rmin=" << rmin
          << ", rmax=" << rmax << " (require 0 < rmin < rmax)";
      throw std::runtime_error(msg.str());
    }

    auto hf = ft["holes_flat"];
    if (!hf || hf.size() == 0 || (hf.size() % 3) != 0) {
      std::ostringstream msg;
      msg << "[RGAFID] 'forward_tagger.holes_flat' must be non-empty with length "
             "multiple of 3: [R1,cx1,cy1, R2,cx2,cy2, ...]";
      throw std::runtime_error(msg.str());
    }

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
        throw std::runtime_error(msg.str());
      }
      u_ft_params.holes.push_back({R, cx, cy});
    }
  }

  // central detector
  {
    auto cvt = top["cvt"];
    if (!cvt) {
      std::ostringstream msg;
      msg << "[RGAFID] Missing required block 'cvt' in " << cfg_path;
      throw std::runtime_error(msg.str());
    }

    if (!cvt["edge_layers"] || cvt["edge_layers"].size() == 0) {
      std::ostringstream msg;
      msg << "[RGAFID] 'cvt.edge_layers' must be a non-empty list";
      throw std::runtime_error(msg.str());
    }
    g_cvt.edge_layers.clear();
    for (auto v : cvt["edge_layers"]) g_cvt.edge_layers.push_back(v.as<int>());

    if (!cvt["edge_min"] || cvt["edge_min"].size() < 1) {
      std::ostringstream msg;
      msg << "[RGAFID] 'cvt.edge_min' must be provided";
      throw std::runtime_error(msg.str());
    }
    g_cvt.edge_min = cvt["edge_min"][0].as<double>();

    if (!cvt["phi_forbidden_deg"] || (cvt["phi_forbidden_deg"].size() % 2) != 0) {
      std::ostringstream msg;
      msg << "[RGAFID] 'cvt.phi_forbidden_deg' must have an even number of values "
             "(pairs of (lo, hi) open intervals)";
      throw std::runtime_error(msg.str());
    }
    g_cvt.phi_forbidden_deg.clear();
    for (auto v : cvt["phi_forbidden_deg"]) g_cvt.phi_forbidden_deg.push_back(v.as<double>());
  }

  // drift chamber (DC)
  {
    auto dc = top["dc"];
    if (!dc) {
      std::ostringstream msg;
      msg << "[RGAFID][VAL] Missing required block 'dc' in " << cfg_path;
      if (m_log) m_log->Error("{}", msg.str());
      throw std::runtime_error(msg.str());
    }

    if (!dc["theta_small_deg"] || dc["theta_small_deg"].size()<1) {
      std::ostringstream msg;
      msg << "[RGAFID][VAL] 'dc.theta_small_deg' must be provided";
      if (m_log) m_log->Error("{}", msg.str());
      throw std::runtime_error(msg.str());
    }
    m_dc_params.theta_small_deg = dc["theta_small_deg"][0].as<double>();

    auto req3 = [&](const char* key, double& e1, double& e2, double& e3){
      if (!dc[key] || dc[key].size()!=3) {
        std::ostringstream msg;
        msg << "[RGAFID][VAL] 'dc."<<key<<"' must be 3 numbers [e1,e2,e3]";
        if (m_log) m_log->Error("{}", msg.str());
        throw std::runtime_error(msg.str());
      }
      e1 = dc[key][0].as<double>();
      e2 = dc[key][1].as<double>();
      e3 = dc[key][2].as<double>();
    };

    // align with Config.yaml and Algorithm.cc
    req3("thresholds_in_smallTheta",
         m_dc_params.in_small_e1, m_dc_params.in_small_e2, m_dc_params.in_small_e3);
    req3("thresholds_in_largeTheta",
         m_dc_params.in_large_e1, m_dc_params.in_large_e2, m_dc_params.in_large_e3);
    req3("thresholds_out",
         m_dc_params.out_e1,      m_dc_params.out_e2,      m_dc_params.out_e3);
  }
}


// USER OVERRIDE
void RGAFiducialFilter::SetStrictness(int strictness) {
  u_strictness_user = strictness;
}


// LIFECYCLE
void RGAFiducialFilter::Start(hipo::banklist& banks)
{
  // Bank presence
  b_particle = GetBankIndex(banks, "REC::Particle");
  if (banklist_has(banks, "RUN::config")) {
    b_config = GetBankIndex(banks, "RUN::config");
  }
  if (banklist_has(banks, "REC::Calorimeter")) { 
    b_calor  = GetBankIndex(banks, "REC::Calorimeter"); m_have_calor = true; 
  }
  if (banklist_has(banks, "REC::ForwardTagger")) { 
    b_ft    = GetBankIndex(banks, "REC::ForwardTagger"); m_have_ft = true; 
  }
  if (banklist_has(banks, "REC::Traj")) { 
    b_traj  = GetBankIndex(banks, "REC::Traj"); m_have_traj = true; 
  }

  // Debug toggles (optional)
  dbg_on     = EnvOn("IGUANA_RGAFID_DBG");
  dbg_ft     = EnvOn("IGUANA_RGAFID_DBG_FT");
  dbg_events = EnvInt("IGUANA_RGAFID_N", 0);

  // load parameters from YAML
  LoadConfigFromYAML();
}

void RGAFiducialFilter::Run(hipo::banklist& banks) const {
  auto& particle = GetBank(banks, b_particle, "REC::Particle");
  auto* cal  = m_have_calor ? &GetBank(banks, b_calor, "REC::Calorimeter") : nullptr;
  auto* ft   = m_have_ft    ? &GetBank(banks, b_ft,    "REC::ForwardTagger") : nullptr;
  auto* traj = m_have_traj  ? &GetBank(banks, b_traj,  "REC::Traj") : nullptr;
  auto& conf = GetBank(banks, b_config, "RUN::config");

  const int ntrk = particle.getRows();

  for (int i = 0; i < ntrk; ++i) {
    const bool pass = Filter(i, particle, conf, cal, ft, traj);
    (void)pass; // decide-only; hook removal if desired
  }
}


// CORE FILTER HELPERS

RGAFiducialFilter::CalLayers
RGAFiducialFilter::CollectCalHitsForTrack(const hipo::bank& cal, int pindex) {
  CalLayers out;
  const int n = cal.getRows();
  for (int i=0; i<n; ++i) {
    if (cal.getInt("pindex", i) != pindex) continue;
    if (cal.getInt("layer",  i) != 1     ) continue; // PCal only
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

bool RGAFiducialFilter::PassCalStrictness(const CalLayers& H, int strictness) {
  // This section still needs further implementation for dead PMTs etc.
  if (!H.has_any) return true; // no PCal -> pass (hadrons etc.)

  float min_lv = std::numeric_limits<float>::infinity();
  float min_lw = std::numeric_limits<float>::infinity();
  for (auto const& hit : H.L1) {
    if (std::isfinite(hit.lv) && hit.lv < min_lv) min_lv = hit.lv;
    if (std::isfinite(hit.lw) && hit.lw < min_lw) min_lw = hit.lw;
  }

  const float thr = (strictness==1 ? 9.0f : strictness==2 ? 13.5f : 18.0f); // default = 1
  return !(min_lv < thr || min_lw < thr);
}

bool RGAFiducialFilter::PassFTFiducial(int pindex, const hipo::bank* ftBank) const {
  if (!ftBank) return true; // no FT -> pass (DC/CVT tracks etc.)

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

bool RGAFiducialFilter::PassCVTFiducial(int pindex, const hipo::bank* trajBank) const {
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

    if (std::find(g_cvt.edge_layers.begin(), g_cvt.edge_layers.end(), layer) != 
      g_cvt.edge_layers.end()) {
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
    if (it == edge_at_layer.end()) continue; // no CVT -> pass (leptons, photons)
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

bool RGAFiducialFilter::PassDCFiducial(int pindex, const hipo::bank& particleBank,
  const hipo::bank& configBank, const hipo::bank* trajBank) const {
  if (!trajBank) return true;

  const int pid = particleBank.getInt("pid", pindex);
  // cuts are defined for inbending and outbending particles separately
  const bool isNeg = (pid== 11 || pid==-211 || pid==-321 || pid==-2212);
  const bool isPos = (pid==-11 || pid== 211 || pid== 321 || pid== 2212);
  if (!(isNeg || isPos)) return true;

  const float torus = configBank.getFloat("torus", 0);
  const bool electron_out = (torus == 1.0);
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
    if (traj.getInt("detector", i) != 6)    continue; // DC, detector == 5 is CVT
    const int layer = traj.getInt("layer", i);
    const double e  = traj.getFloat("edge", i);
    if      (layer== 6) e1 = e;
    else if (layer==18) e2 = e;
    else if (layer==36) e3 = e;
  }

  auto pass3 = [](double e1, double e2, double e3, double t1, double t2, double t3)->bool {
    return (e1>t1 && e2>t2 && e3>t3);
  };

  if (particle_inb) {
    if (theta < g_dc.theta_small_deg) {
      return pass3(e1,e2,e3, g_dc.in_small_e1, g_dc.in_small_e2, g_dc.in_small_e3);
    }
    return pass3(e1,e2,e3, g_dc.in_large_e1, g_dc.in_large_e2, g_dc.in_large_e3);
  } else if (particle_out) {
    return pass3(e1,e2,e3, g_dc.out_e1, g_dc.out_e2, g_dc.out_e3);
  }
  return true;
}

bool RGAFiducialFilter::Filter(int track_index, const hipo::bank& particleBank,
  const hipo::bank& configBank, const hipo::bank* calBank, const hipo::bank* ftBank, 
  const hipo::bank* trajBank) const {
  const int pid = particleBank.getInt("pid", track_index);

  const int strictness = u_strictness_user.value_or(g_yaml_cal_strictness);

  bool pass = true;

  if (pid == 11 || pid == 22) {
    // photons and electrons are detected in either (exclusive) FT or FD
    if (calBank) {
      auto calhits = CollectCalHitsForTrack(*calBank, track_index);
      pass = pass && PassCalStrictness(calhits, strictness);
    }
    pass = pass && PassFTFiducial(track_index, ftBank);
  }

  if (pid== 211 || pid== 321 || pid== 2212 ||
      pid==-211 || pid==-321 || pid==-2212) {
    pass = pass && PassCVTFiducial(track_index, trajBank);
    pass = pass && PassDCFiducial(track_index, particleBank, configBank, trajBank);
  }

  return pass;
}

} 