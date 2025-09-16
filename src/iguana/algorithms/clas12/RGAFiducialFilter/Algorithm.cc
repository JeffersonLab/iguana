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
#include <vector>

// Default settings defined and adjustable in Config.yaml (CAL)
//
// * Forward Tagger cut on annulus and missing holes for pid==11,22
//
// * Calorimeter cut on lv,lw>9,13.5,18 depending on SetStrictness([default = 1]) for pid==11,22; 
//    Strictness 1 loosely recommended for spin-asymmetries, cross sections may want to consider
//    higher strictness as well as ...
//    !!! TO DO: implement run-by-run data/MC matching for missing PMTs, etc.
//
// * Central Detector cut on areas between 3 sectors and require tracks inside detector (edge > 0);
//    for all charged hadrons
// 
// * Drift chamber cut on region 1 > 3 cm, region 2 > 3, region3 > 10; inbending tracks also require
//    region 1 > 10, region 2 > 10 for theta < 10; cuts for all charged leptons and hadrons

namespace iguana::clas12 {
  REGISTER_IGUANA_ALGORITHM(RGAFiducialFilter, "clas12::RGAFiducialFilter");

  static bool banklist_has(hipo::banklist& banks, const char* name) {
    for (auto& b : banks) if (b.getSchema().getName() == name) return true;
    return false;
  }

  // Build path to YAML 
  static inline std::string GetAlgConfigPath() {
    // 1) Prefer a Config.yaml sitting next to this source file
    std::string here = __FILE__;
    auto slash = here.find_last_of("/\\");
    std::string src_dir = (slash == std::string::npos) ? "." : here.substr(0, slash);
    const std::string local = src_dir + "/Config.yaml";

    // 2) Fallback: repo-relative path (running from build tree)
    const std::string repo_rel =
        "src/iguana/algorithms/clas12/RGAFiducialFilter/Config.yaml";

    // Helper to add candidate paths safely
    auto add = [](std::vector<std::string>& v, const std::string& base) {
      if (base.empty()) return;
      v.push_back(base + "/algorithms/clas12/RGAFiducialFilter/Config.yaml");   // current
    };

    std::vector<std::string> candidates;
    candidates.push_back(local);
    candidates.push_back(repo_rel);

    // 3) IGUANA_CONFIG_PATH: colon-separated search roots 
    if (const char* icp = std::getenv("IGUANA_CONFIG_PATH")) {
      std::string s(icp);
      size_t start = 0;
      while (true) {
        size_t sep = s.find(':', start);
        std::string dir = (sep == std::string::npos) ? s.substr(start) : s.substr(start, sep - start);
        add(candidates, dir);
        if (sep == std::string::npos) break;
        start = sep + 1;
      }
    }

    // 4) IGUANA_ETCDIR: env var or compiled-in default (meson defines IGUANA_ETCDIR)
    std::string base_etc;
    if (const char* env_etc = std::getenv("IGUANA_ETCDIR")) {
      base_etc = env_etc;
    } else {
  #ifdef IGUANA_ETCDIR
      base_etc = IGUANA_ETCDIR;  // e.g. "/prefix/etc/iguana"
  #endif
    }
    add(candidates, base_etc);

    // Try candidates in order
    for (const auto& p : candidates) {
      std::ifstream f(p);
      if (f.good()) return p;
    }

    // If none exist, return the most likely install path for a clear error message
    return !base_etc.empty()
             ? (base_etc + "/algorithms/clas12/RGAFiducialFilter/Config.yaml")
             : local;
  }

  // YAML loading (required)
  void RGAFiducialFilter::LoadConfigFromYAML() {

    // Ensure YAML parsed at least once (older API: no IsEmpty())
    if (!GetConfig()) {
      ParseYAMLConfig();
    }
    const char* TOP = "clas12::RGAFiducialFilter";

    // ---------------------------
    // calorimeter.strictness
    // ---------------------------
    // Expect a 1-element list
    {
      auto v = GetOptionVector<int>("calorimeter.strictness",
        {TOP, "calorimeter", "strictness"});
      if (v.empty()) {
        throw std::runtime_error("[RGAFID] Missing 'calorimeter.strictness' (need [1|2|3])");
      }
      m_cal_strictness = v[0];
    }
    if (m_cal_strictness < 1 || m_cal_strictness > 3) {
      std::ostringstream msg;
      msg << "[RGAFID] 'calorimeter.strictness' must be 1, 2, or 3 (got "
          << m_cal_strictness << ")";
      throw std::runtime_error(msg.str());
    }

    // ---------------------------
    // forward_tagger.{radius, holes_flat}
    // ---------------------------
    {
      // radius must be [rmin, rmax]
      auto radius = GetOptionVector<float>("forward_tagger.radius", 
        {TOP, "forward_tagger", "radius"});
      if (radius.size() != 2) {
        throw std::runtime_error(
            "[RGAFID] 'forward_tagger.radius' must be a 2-element list [rmin, rmax]");
      }
      float rmin = radius[0];
      float rmax = radius[1];
      if (!(std::isfinite(rmin) && std::isfinite(rmax)) || !(rmin > 0.f && rmax > rmin)) {
        std::ostringstream msg;
        msg << "[RGAFID] Invalid 'forward_tagger.radius': rmin=" << rmin
            << ", rmax=" << rmax << " (require 0 < rmin < rmax)";
        throw std::runtime_error(msg.str());
      }

      // holes_flat must be length multiple of 3: [R1, cx1, cy1, R2, cx2, cy2, ...]
      auto holes_flat = GetOptionVector<float>("forward_tagger.holes_flat",
                                               {TOP, "forward_tagger", "holes_flat"});
      if (holes_flat.empty() || (holes_flat.size() % 3) != 0) {
        throw std::runtime_error(
            "[RGAFID] 'forward_tagger.holes_flat' must be non-empty with length multiple of 3: "
            "[R1,cx1,cy1, R2,cx2,cy2, ...]");
      }

      u_ft_params.rmin = rmin;
      u_ft_params.rmax = rmax;
      u_ft_params.holes.clear();
      u_ft_params.holes.reserve(holes_flat.size() / 3);
      for (std::size_t i = 0; i < holes_flat.size(); i += 3) {
        float R  = holes_flat[i + 0];
        float cx = holes_flat[i + 1];
        float cy = holes_flat[i + 2];
        if (!(std::isfinite(R) && std::isfinite(cx) && std::isfinite(cy)) || R <= 0.f) {
          std::ostringstream msg;
          msg << "[RGAFID] Invalid FT hole triple at index " << (i / 3)
              << " -> (R=" << R << ", cx=" << cx << ", cy=" << cy << ")";
          throw std::runtime_error(msg.str());
        }
        u_ft_params.holes.push_back({R, cx, cy});
      }
    }

    // ---------------------------
    // cvt.{edge_layers, edge_min[0], phi_forbidden_deg}
    // ---------------------------
    {
      auto edge_layers = GetOptionVector<int>("cvt.edge_layers",
                                              {TOP, "cvt", "edge_layers"});
      if (edge_layers.empty()) {
        throw std::runtime_error("[RGAFID] 'cvt.edge_layers' must be a non-empty list");
      }
      m_cvt.edge_layers = edge_layers;

      // edge_min provided as a 1-element list; read element 0
      {
        auto v = GetOptionVector<double>("cvt.edge_min", {TOP, "cvt", "edge_min"});
        if (v.empty()) {
          throw std::runtime_error("[RGAFID] 'cvt.edge_min' must be provided as [value]");
        }
        m_cvt.edge_min = v[0];
      }

      // phi_forbidden_deg must be an even-length list of (lo,hi) pairs
      auto phi_forbidden = GetOptionVector<double>("cvt.phi_forbidden_deg",
                                                   {TOP, "cvt", "phi_forbidden_deg"});
      if ((phi_forbidden.size() % 2) != 0) {
        throw std::runtime_error(
            "[RGAFID] 'cvt.phi_forbidden_deg' must have an even number of values "
            "(pairs of (lo, hi) open intervals)");
      }
      m_cvt.phi_forbidden_deg = std::move(phi_forbidden);
    }

    // ---------------------------
    // dc.{theta_small_deg[0], thresholds_*}
    // ---------------------------
    {
      // theta_small_deg is a 1-element list; read element 0
      {
        auto v = GetOptionVector<double>("dc.theta_small_deg",
                                         {TOP, "dc", "theta_small_deg"});
        if (v.empty()) {
          throw std::runtime_error("[RGAFID] 'dc.theta_small_deg' must be provided as [value]");
        }
        m_dc.theta_small_deg = v[0];
      }

      auto need3 = [&](const char* key) -> std::vector<double> {
        auto v = GetOptionVector<double>(std::string("dc.") + key, {TOP, "dc", key});
        if (v.size() != 3) {
          std::ostringstream msg;
          msg << "[RGAFID] 'dc." << key << "' must be a 3-element list [e1,e2,e3]";
          throw std::runtime_error(msg.str());
        }
        return v;
      };

      {
        auto thr = need3("thresholds_out");
        m_dc.out_e1 = thr[0]; m_dc.out_e2 = thr[1]; m_dc.out_e3 = thr[2];
      }
      {
        auto thr = need3("thresholds_in_smallTheta");
        m_dc.in_small_e1 = thr[0]; m_dc.in_small_e2 = thr[1]; m_dc.in_small_e3 = thr[2];
      }
      {
        auto thr = need3("thresholds_in_largeTheta");
        m_dc.in_large_e1 = thr[0]; m_dc.in_large_e2 = thr[1]; m_dc.in_large_e3 = thr[2];
      }
    }
  }


  // user override for strictness
  void RGAFiducialFilter::SetStrictness(int strictness) {
    u_strictness_user = strictness;
  }


  // start main filter
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

    // load parameters from YAML
    LoadConfigFromYAML();
  }

  void RGAFiducialFilter::Run(hipo::banklist& banks) const {
    // Grab the banks needed for this event
    auto& particle = GetBank(banks, b_particle, "REC::Particle");
    auto& conf     = GetBank(banks, b_config,   "RUN::config");
    auto* cal      = m_have_calor ? &GetBank(banks, b_calor, "REC::Calorimeter")   : nullptr;
    auto* ft       = m_have_ft    ? &GetBank(banks, b_ft,    "REC::ForwardTagger") : nullptr;
    auto* traj     = m_have_traj  ? &GetBank(banks, b_traj,  "REC::Traj")          : nullptr;

    // Prune in place: keep only rows that pass fiducial logic
    particle.getMutableRowList().filter([&](auto /*bank*/, auto row) {
      const bool keep = Filter(static_cast<int>(row), particle, conf, cal, ft, traj);
      return keep ? 1 : 0;   // 1 = keep row, 0 = drop row
    });
  }


  // core helpers
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
    if (!H.has_any) return true; // no PCal track -> pass (hadrons etc.)

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
    if (!ftBank) return true; // no FT track -> pass (DC/CVT tracks etc.)

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
      if (traj.getInt("detector", i) != 5     ) continue; // CVT is detector == 5

      const int layer = traj.getInt("layer", i);
      const double e  = traj.getFloat("edge", i);

      if (std::find(m_cvt.edge_layers.begin(), m_cvt.edge_layers.end(), layer) != 
        m_cvt.edge_layers.end()) {
        edge_at_layer[layer] = e;
      }
      if (layer == 12) {
        x12 = traj.getFloat("x", i);
        y12 = traj.getFloat("y", i);
        saw12 = true;
      }
    }

    for (int L : m_cvt.edge_layers) {
      auto it = edge_at_layer.find(L);
      if (it == edge_at_layer.end()) continue; // no CVT track -> pass (leptons, CAL photons)
      if (!(it->second > m_cvt.edge_min)) return false;
    }

    if (saw12 && !m_cvt.phi_forbidden_deg.empty()) {
      double phi = std::atan2(y12, x12) * (180.0/M_PI);
      if (phi < 0) phi += 360.0;
      for (std::size_t i=0; i+1<m_cvt.phi_forbidden_deg.size(); i+=2) {
        const double lo = m_cvt.phi_forbidden_deg[i];
        const double hi = m_cvt.phi_forbidden_deg[i+1];
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
    // the CLAS convention is to call the torus polarity inbending or outbending depending on the
    // curvature of electrons, e.g. in "inbending" data pi+ is an outbending track
    const bool isNeg = (pid== 11 || pid==-211 || pid==-321 || pid==-2212);
    const bool isPos = (pid==-11 || pid== 211 || pid== 321 || pid== 2212);
    if (!(isNeg || isPos)) return true; // photon, neutron or unassigned by EventBuilder

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
      if (traj.getInt("detector", i) != 6)    continue; // DC == 6, detector == 5 would be CVT
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
      if (theta < m_dc.theta_small_deg) {
        return pass3(e1,e2,e3, m_dc.in_small_e1, m_dc.in_small_e2, m_dc.in_small_e3);
      }
      return pass3(e1,e2,e3, m_dc.in_large_e1, m_dc.in_large_e2, m_dc.in_large_e3);
    } else if (particle_out) {
      return pass3(e1,e2,e3, m_dc.out_e1, m_dc.out_e2, m_dc.out_e3);
    }
    return true;
  }

  bool RGAFiducialFilter::Filter(
    int track_index,
    const hipo::bank& particleBank,
    const hipo::bank& configBank,
    const hipo::bank* calBank,
    const hipo::bank* ftBank,
    const hipo::bank* trajBank) const
  {
    const int pid = particleBank.getInt("pid", track_index);
    const int strictness = u_strictness_user.value_or(m_cal_strictness);

    auto has_assoc = [&](const hipo::bank* b)->bool {
      if (!b) return false;
      const int n = b->getRows();
      for (int i=0;i<n;++i) if (b->getInt("pindex", i) == track_index) return true;
      return false;
    };

    const bool hasCal = has_assoc(calBank);
    const bool hasFT  = has_assoc(ftBank);

    bool pass = true;

    if (pid == 11) {
      // Electron: FT topology -> FT cut; else -> CAL + DC
      if (hasFT) {
        pass = pass && PassFTFiducial(track_index, ftBank);
      } else {
        if (hasCal) {
          auto calhits = CollectCalHitsForTrack(*calBank, track_index);
          pass = pass && PassCalStrictness(calhits, strictness);
        }
        pass = pass && PassDCFiducial(track_index, particleBank, configBank, trajBank);
      }
      return pass;
    }

    if (pid == 22) {
      // Photon: FT or CAL, whichever is associated (don’t AND them)
      if (hasFT) {
        pass = pass && PassFTFiducial(track_index, ftBank);
      } else if (hasCal) {
        auto calhits = CollectCalHitsForTrack(*calBank, track_index);
        pass = pass && PassCalStrictness(calhits, strictness);
      }
      return pass;
    }

    // Charged hadrons
    if (pid== 211 || pid== 321 || pid== 2212 ||
        pid==-211 || pid==-321 || pid==-2212) {
      pass = pass && PassCVTFiducial(track_index, trajBank);
      pass = pass && PassDCFiducial(track_index, particleBank, configBank, trajBank);
      return pass;
    }

    // neutrals/others
    return true;
  }

} 