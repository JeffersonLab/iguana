// src/iguana/algorithms/clas12/RGAFiducialFilter/Algorithm.cc
#include "Algorithm.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <map>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace iguana::clas12 {

REGISTER_IGUANA_ALGORITHM(RGAFiducialFilter, "clas12::RGAFiducialFilter");

// small helpers
static bool banklist_has(hipo::banklist& banks, const char* name) {
  for (auto& b : banks) if (b.getSchema().getName() == name) return true;
  return false;
}

static bool traj_has_detector(const hipo::bank* trajBank, int pindex, int detector) {
  if (!trajBank) return false;
  const auto& traj = *trajBank;
  const int n = traj.getRows();
  for (int i=0; i<n; ++i) {
    if (traj.getInt("pindex", i) == pindex && traj.getInt("detector", i) == detector) {
      return true;
    }
  }
  return false;
}

// configuration
void RGAFiducialFilter::LoadConfig() {
  // Calorimeter strictness (required: 1,2,3)
  m_cal_strictness = GetOptionScalar<int>("calorimeter.strictness", {"calorimeter", "strictness"});
  if (m_cal_strictness < 1 || m_cal_strictness > 3) {
    throw std::runtime_error("[RGAFID] 'calorimeter.strictness' must be 1, 2, or 3");
  }

  // Forward Tagger (required: radius; optional: holes_flat)
  {
    auto radius = GetOptionVector<double>("forward_tagger.radius", {"forward_tagger","radius"});
    if (radius.size() != 2) {
      throw std::runtime_error("[RGAFID] 'forward_tagger.radius' must be [rmin, rmax]");
    }
    u_ft_params.rmin = static_cast<float>(radius[0]);
    u_ft_params.rmax = static_cast<float>(radius[1]);
    if (!(std::isfinite(u_ft_params.rmin) && std::isfinite(u_ft_params.rmax)) ||
        !(u_ft_params.rmin > 0.f && u_ft_params.rmax > u_ft_params.rmin)) {
      throw std::runtime_error("[RGAFID] invalid forward_tagger.radius values");
    }

    u_ft_params.holes.clear();
    try {
      auto holes_flat = GetOptionVector<double>("forward_tagger.holes_flat",
                                                {"forward_tagger","holes_flat"});
      if (!holes_flat.empty() && (holes_flat.size() % 3) != 0) {
        throw std::runtime_error("[RGAFID] 'forward_tagger.holes_flat' must have 3N values");
      }
      u_ft_params.holes.reserve(holes_flat.size()/3);
      for (std::size_t i=0; i+2<holes_flat.size(); i+=3) {
        float R  = static_cast<float>(holes_flat[i+0]);
        float cx = static_cast<float>(holes_flat[i+1]);
        float cy = static_cast<float>(holes_flat[i+2]);
        if (!(std::isfinite(R) && std::isfinite(cx) && std::isfinite(cy)) || R<=0.f) {
          throw std::runtime_error("[RGAFID] invalid FT hole triple in 'holes_flat'");
        }
        u_ft_params.holes.push_back({R,cx,cy});
      }
    } catch (const std::exception&) {
      
    }
  }

  // CVT (required: edge_layers, edge_min; optional: phi_forbidden_deg)
  {
    m_cvt.edge_layers = GetOptionVector<int>("cvt.edge_layers", {"cvt","edge_layers"});
    if (m_cvt.edge_layers.empty()) {
      throw std::runtime_error("[RGAFID] 'cvt.edge_layers' must be non-empty");
    }
    m_cvt.edge_min = GetOptionScalar<double>("cvt.edge_min", {"cvt","edge_min"});

    m_cvt.phi_forbidden_deg.clear();
    try {
      m_cvt.phi_forbidden_deg =
        GetOptionVector<double>("cvt.phi_forbidden_deg", {"cvt","phi_forbidden_deg"});
      if (!m_cvt.phi_forbidden_deg.empty() && (m_cvt.phi_forbidden_deg.size() % 2) != 0) {
        throw std::runtime_error("[RGAFID] 'cvt.phi_forbidden_deg' must have pairs (2N values)");
      }
    } catch (const std::exception&) {

    }
  }

  // DC (required: theta_small_deg and three threshold triplets)
  {
    m_dc.theta_small_deg =
      GetOptionScalar<double>("dc.theta_small_deg", {"dc","theta_small_deg"});

    auto need3 = [&](const char* key) -> std::array<double,3> {
      auto v = GetOptionVector<double>(std::string("dc.")+key, {"dc", key});
      if (v.size() != 3) {
        throw std::runtime_error(std::string("[RGAFID] 'dc.") + key + "' must be [e1,e2,e3]");
      }
      return {v[0], v[1], v[2]};
    };

    auto out  = need3("thresholds_out");
    auto in_s = need3("thresholds_in_smallTheta");
    auto in_l = need3("thresholds_in_largeTheta");

    m_dc.out_e1 = out[0];  m_dc.out_e2 = out[1];  m_dc.out_e3 = out[2];
    m_dc.in_small_e1 = in_s[0]; m_dc.in_small_e2 = in_s[1]; m_dc.in_small_e3 = in_s[2];
    m_dc.in_large_e1 = in_l[0]; m_dc.in_large_e2 = in_l[1]; m_dc.in_large_e3 = in_l[2];
  }
}

// lifecycle
void RGAFiducialFilter::Start(hipo::banklist& banks)
{
  b_particle = GetBankIndex(banks, "REC::Particle");

  if (banklist_has(banks, "RUN::config")) {
    b_config = GetBankIndex(banks, "RUN::config");
  }
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

  ParseYAMLConfig();
  LoadConfig();
}

void RGAFiducialFilter::Run(hipo::banklist& banks) const {
  auto& particle = GetBank(banks, b_particle, "REC::Particle");
  auto& conf     = GetBank(banks, b_config,   "RUN::config");
  auto* cal      = m_have_calor ? &GetBank(banks, b_calor, "REC::Calorimeter")   : nullptr;
  auto* ft       = m_have_ft    ? &GetBank(banks, b_ft,    "REC::ForwardTagger") : nullptr;
  auto* traj     = m_have_traj  ? &GetBank(banks, b_traj,  "REC::Traj")          : nullptr;

  // prune in place
  particle.getMutableRowList().filter([&](auto, auto row) {
    const bool keep = Filter(static_cast<int>(row), particle, conf, cal, ft, traj);
    return keep ? 1 : 0;
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
  if (!H.has_any) return true;

  float min_lv = std::numeric_limits<float>::infinity();
  float min_lw = std::numeric_limits<float>::infinity();
  for (auto const& hit : H.L1) {
    if (std::isfinite(hit.lv) && hit.lv < min_lv) min_lv = hit.lv;
    if (std::isfinite(hit.lw) && hit.lw < min_lw) min_lw = hit.lw;
  }

  const float thr = (strictness==1 ? 9.0f : strictness==2 ? 13.5f : 18.0f);
  return !(min_lv < thr || min_lw < thr);
}

bool RGAFiducialFilter::PassFTFiducial(int pindex, const hipo::bank* ftBank) const {
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
    return true; // first associated FT hit 
  }
  return true; // no FT association
}

bool RGAFiducialFilter::PassCVTFiducial(int pindex, const hipo::bank* trajBank) const {
  if (!trajBank) return true;

  const auto& traj = *trajBank;
  const int n = traj.getRows();

  // One edge value per required layer for this track 
  std::map<int, double> edge_at_layer;

  double x12 = 0.0, y12 = 0.0;
  bool saw12 = false;

  for (int i = 0; i < n; ++i) {
    if (traj.getInt("pindex", i)   != pindex) continue;
    if (traj.getInt("detector", i) != 5     ) continue; // CVT

    const int layer   = traj.getInt("layer", i);
    const double edge = static_cast<double>(traj.getFloat("edge", i));

    if (std::find(m_cvt.edge_layers.begin(), m_cvt.edge_layers.end(), layer) != m_cvt.edge_layers.end()) {
      edge_at_layer[layer] = edge;
    }

    if (layer == 12) {
      const double x = static_cast<double>(traj.getFloat("x", i));
      const double y = static_cast<double>(traj.getFloat("y", i));
      if (std::isfinite(x) && std::isfinite(y)) {
        x12 = x; y12 = y; saw12 = true;
      }
    }
  }

  // Apply edge > edge_min for each layer that has a measurement; missing layer -> pass.
  for (int L : m_cvt.edge_layers) {
    auto it = edge_at_layer.find(L);
    if (it == edge_at_layer.end()) continue;            // missing layer -> pass
    if (!(it->second > m_cvt.edge_min)) {               // fail if edge <= edge_min
      return false;
    }
  }

  // Phi veto from YAML 
  if (saw12 && !m_cvt.phi_forbidden_deg.empty()) {
    constexpr double kPI = 3.141593;
    double phi = std::atan2(y12, x12) * (180.0 / kPI);
    if (phi < 0) phi += 360.0;
    for (std::size_t i = 0; i + 1 < m_cvt.phi_forbidden_deg.size(); i += 2) {
      const double lo = m_cvt.phi_forbidden_deg[i];
      const double hi = m_cvt.phi_forbidden_deg[i + 1];
      if (phi > lo && phi < hi) {
        return false;
      }
    }
  }

  return true;
}

bool RGAFiducialFilter::PassDCFiducial(int pindex, const hipo::bank& particleBank,
    const hipo::bank& configBank, const hipo::bank* trajBank) const {
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
  const double kPI = 3.141593;
  const double theta = std::atan2(rho, (pz==0.0 ? 1e-12 : pz)) * (180.0 / kPI);

  double e1=0.0, e2=0.0, e3=0.0;
  bool   saw_dc = false;

  const auto& traj = *trajBank;
  const int n = traj.getRows();
  for (int i=0; i<n; ++i) {
    if (traj.getInt("pindex", i) != pindex) continue;
    if (traj.getInt("detector", i) != 6)    continue; // DC
    saw_dc = true;
    const int layer = traj.getInt("layer", i);
    const double e  = traj.getFloat("edge", i);
    if      (layer== 6) e1 = e;
    else if (layer==18) e2 = e;
    else if (layer==36) e3 = e;
  }

  // if track has no DC rows at all, DC cuts do not apply.
  if (!saw_dc) return true;

  auto pass3 = [](double a1, double a2, double a3, double t1, double t2, double t3)->bool {
    return (a1>t1 && a2>t2 && a3>t3);
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

// per-track decision
bool RGAFiducialFilter::Filter(int track_index, const hipo::bank& particleBank,
  const hipo::bank& configBank, const hipo::bank* calBank,
  const hipo::bank* ftBank, const hipo::bank* trajBank) const {

  const int pid = particleBank.getInt("pid", track_index);
  const int strictness = m_cal_strictness; 

  auto has_assoc = [&](const hipo::bank* b)->bool {
    if (!b) return false;
    const int n = b->getRows();
    for (int i=0;i<n;++i) if (b->getInt("pindex", i) == track_index) return true;
    return false;
  };

  const bool hasCal = has_assoc(calBank);
  const bool hasFT  = has_assoc(ftBank);
  const bool hasCVT = traj_has_detector(trajBank, track_index, 5);
  const bool hasDC  = traj_has_detector(trajBank, track_index, 6);

  bool pass = true;

  if (pid == 11) { // electron
    if (hasFT) {
      pass = pass && PassFTFiducial(track_index, ftBank);
    } else {
      if (hasCal) {
        auto calhits = CollectCalHitsForTrack(*calBank, track_index);
        pass = pass && PassCalStrictness(calhits, strictness);
      }
      if (hasDC) {
        pass = pass && PassDCFiducial(track_index, particleBank, configBank, trajBank);
      }
    }
    return pass;
  }

  if (pid == 22) { // photon
    if (hasFT) {
      pass = pass && PassFTFiducial(track_index, ftBank);
    } else if (hasCal) {
      auto calhits = CollectCalHitsForTrack(*calBank, track_index);
      pass = pass && PassCalStrictness(calhits, strictness);
    }
    return pass;
  }

  // charged hadrons
  if (pid== 211 || pid== 321 || pid== 2212 ||
      pid==-211 || pid==-321 || pid==-2212) {
    // only apply the cut for the subsystem the track actually has
    if (hasCVT) pass = pass && PassCVTFiducial(track_index, trajBank);
    if (hasDC)  pass = pass && PassDCFiducial(track_index, particleBank, configBank, trajBank);
    return pass;
  }

  // neutrals/others: no cut
  return true;
}

} 