// src/iguana/algorithms/clas12/RGAFiducialFilter/Algorithm.cc

#include "Algorithm.h"
#include "iguana/services/YAMLReader.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>
#include <limits>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace iguana::clas12 {

REGISTER_IGUANA_ALGORITHM(RGAFiducialFilter);

// ------------------------------------------------------------
// Small helpers
// ------------------------------------------------------------
static bool banklist_has(hipo::banklist& banks, const char* name) {
  for (auto& b : banks) if (b.getSchema().getName() == name) return true;
  return false;
}

// Keep the last path we actually loaded for better error messages
static std::string g_rgafid_yaml_path;


void RGAFiducialFilter::Start(hipo::banklist& banks)
{
  // Get configuration using Iguana's system
  ParseYAMLConfig();
  
  // Discover available banks
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

  // Load configuration
  LoadConfig();
}

void RGAFiducialFilter::LoadConfig() {
  // Get the configuration node for this algorithm
  auto config = GetConfig();
  
  // --- Calorimeter
  m_cal_strictness = config->GetOption<int>("calorimeter/strictness");
  if (m_cal_strictness < 1 || m_cal_strictness > 3)
    throw std::runtime_error("[RGAFID] 'calorimeter.strictness' must be 1,2,3");

  // --- Forward Tagger
  auto radius = config->GetOptionVector<double>("forward_tagger/radius");
  if (radius.size() != 2)
    throw std::runtime_error("[RGAFID] 'forward_tagger.radius' must be [rmin,rmax]");
  
  u_ft_params.rmin = static_cast<float>(radius[0]);
  u_ft_params.rmax = static_cast<float>(radius[1]);
  
  if (config->HasOption("forward_tagger/holes_flat")) {
    auto holes_flat = config->GetOptionVector<double>("forward_tagger/holes_flat");
    if (!holes_flat.empty() && (holes_flat.size() % 3) != 0)
      throw std::runtime_error("[RGAFID] 'forward_tagger.holes_flat' must have 3N values");
    
    u_ft_params.holes.clear();
    u_ft_params.holes.reserve(holes_flat.size()/3);
    for (std::size_t i=0; i+2<holes_flat.size(); i+=3) {
      float R  = static_cast<float>(holes_flat[i+0]);
      float cx = static_cast<float>(holes_flat[i+1]);
      float cy = static_cast<float>(holes_flat[i+2]);
      if (!(std::isfinite(R) && std::isfinite(cx) && std::isfinite(cy)) || R<=0.f)
        throw std::runtime_error("[RGAFID] invalid FT hole triple in 'holes_flat'");
      u_ft_params.holes.push_back({R,cx,cy});
    }
  }

  // --- CVT
  m_cvt.edge_layers = config->GetOptionVector<int>("cvt/edge_layers");
  if (m_cvt.edge_layers.empty())
    throw std::runtime_error("[RGAFID] 'cvt.edge_layers' must be non-empty");

  m_cvt.edge_min = config->GetOption<double>("cvt/edge_min");

  if (config->HasOption("cvt/phi_forbidden_deg")) {
    m_cvt.phi_forbidden_deg = config->GetOptionVector<double>("cvt/phi_forbidden_deg");
    if (!m_cvt.phi_forbidden_deg.empty() && (m_cvt.phi_forbidden_deg.size() % 2) != 0)
      throw std::runtime_error("[RGAFID] 'cvt.phi_forbidden_deg' must have pairs (2N values)");
  }

  // --- DC
  m_dc.theta_small_deg = config->GetOption<double>("dc/theta_small_deg");

  auto read_thresholds = [&](const std::string& key) -> std::array<double, 3> {
    auto v = config->GetOptionVector<double>(key);
    if (v.size() != 3) {
      throw std::runtime_error("[RGAFID] 'dc." + key + "' must be [e1,e2,e3]");
    }
    return {v[0], v[1], v[2]};
  };

  auto out = read_thresholds("dc/thresholds_out");
  auto in_s = read_thresholds("dc/thresholds_in_smallTheta");
  auto in_l = read_thresholds("dc/thresholds_in_largeTheta");

  m_dc.out_e1 = out[0];  m_dc.out_e2 = out[1];  m_dc.out_e3 = out[2];
  m_dc.in_small_e1 = in_s[0]; m_dc.in_small_e2 = in_s[1]; m_dc.in_small_e3 = in_s[2];
  m_dc.in_large_e1 = in_l[0]; m_dc.in_large_e2 = in_l[1]; m_dc.in_large_e3 = in_l[2];
}

void RGAFiducialFilter::Run(hipo::banklist& banks) const {
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

// ------------------------------------------------------------
// Core helpers
// ------------------------------------------------------------
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
  if (!H.has_any) return true; // no PCal track -> pass

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
  if (!ftBank) return true; // no FT track -> pass

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
    if (traj.getInt("detector", i) != 5     ) continue; // CVT == 5

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
    if (it == edge_at_layer.end()) continue; // missing layer -> pass
    if (!(it->second > m_cvt.edge_min)) return false;
  }

  constexpr double kPI = 3.14159265358979323846;
  if (saw12 && !m_cvt.phi_forbidden_deg.empty()) {
    double phi = std::atan2(y12, x12) * (180.0 / kPI);
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
                                       const hipo::bank& configBank,
                                       const hipo::bank* trajBank) const {
  if (!trajBank) return true;

  const int pid = particleBank.getInt("pid", pindex);
  // cuts are defined for inbending and outbending particles separately
  const bool isNeg = (pid== 11 || pid==-211 || pid==-321 || pid==-2212);
  const bool isPos = (pid==-11 || pid== 211 || pid== 321 || pid== 2212);
  if (!(isNeg || isPos)) return true; // photon, neutron or unassigned

  const float torus = configBank.getFloat("torus", 0);
  const bool electron_out = (torus == 1.0f);
  const bool particle_inb = (electron_out ? isPos : isNeg);
  const bool particle_out = !particle_inb;

  const double px = particleBank.getFloat("px", pindex);
  const double py = particleBank.getFloat("py", pindex);
  const double pz = particleBank.getFloat("pz", pindex);
  const double rho = std::hypot(px, py);
  const double theta = std::atan2(rho, (pz==0.0 ? 1e-12 : pz)) * (180.0 / 3.14159265358979323846);

  double e1=0.0, e2=0.0, e3=0.0;
  const auto& traj = *trajBank;
  const int n = traj.getRows();
  for (int i=0; i<n; ++i) {
    if (traj.getInt("pindex", i) != pindex) continue;
    if (traj.getInt("detector", i) != 6)    continue; // DC == 6
    const int layer = traj.getInt("layer", i);
    const double e  = traj.getFloat("edge", i);
    if      (layer== 6) e1 = e;
    else if (layer==18) e2 = e;
    else if (layer==36) e3 = e;
  }

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

// ------------------------------------------------------------
// Filter (per-track)
// ------------------------------------------------------------
bool RGAFiducialFilter::Filter(int track_index,
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

} // namespace iguana::clas12