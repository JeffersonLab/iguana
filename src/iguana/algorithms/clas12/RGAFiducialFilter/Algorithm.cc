#include "Algorithm.h"
#include "iguana/services/YAMLReader.h"   // YAMLReader::node_path_t for GetOptionVector
#include <yaml-cpp/yaml.h>

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

REGISTER_IGUANA_ALGORITHM(RGAFiducialFilter, "clas12::RGAFiducialFilter");

// Build YAML node paths for GetOptionVector (expects node_path_t)
using NodePath = iguana::YAMLReader::node_path_t;
static inline NodePath Path(std::initializer_list<const char*> keys) {
  NodePath p;
  for (auto* k : keys) p.emplace_back(std::string(k));
  return p;
}

static bool banklist_has(hipo::banklist& banks, const char* name) {
  for (auto& b : banks) if (b.getSchema().getName() == name) return true;
  return false;
}

// Load the RGAFiducialFilter YAML once and return the effective root:
// --- keep the existing includes and RGAFID_LoadRootYAML() ---

// Walk to a node under the RGAFiducialFilter root; throw with a helpful key name.
static YAML::Node RGAFID_GetNode(std::initializer_list<const char*> keys,
                                 const char* dbgkey_for_errors) {
  YAML::Node node = RGAFID_LoadRootYAML();
  for (auto* k : keys) {
    if (!node[k]) {
      std::ostringstream msg;
      msg << "[RGAFID] Missing key '" << k << "' while reading " << dbgkey_for_errors;
      throw std::runtime_error(msg.str());
    }
    node = node[k];
  }
  return node;
}

// Read a vector<T>. If the YAML provides a scalar, accept it as {value} (1-element vector).
template <typename T>
static std::vector<T> RGAFID_ReadVector(std::initializer_list<const char*> keys,
                                        const char* dbgkey_for_errors) {
  YAML::Node node = RGAFID_GetNode(keys, dbgkey_for_errors);
  std::vector<T> out;
  if (node.IsSequence()) {
    out.reserve(node.size());
    for (auto const& item : node) {
      try { out.push_back(item.as<T>()); }
      catch (const std::exception& e) {
        std::ostringstream msg; msg << "[RGAFID] Vector conversion failed for "
                                    << dbgkey_for_errors << ": " << e.what();
        throw std::runtime_error(msg.str());
      }
    }
    return out;
  }
  if (node.IsScalar()) {
    try { out.push_back(node.as<T>()); return out; }
    catch (const std::exception& e) {
      std::ostringstream msg; msg << "[RGAFID] Scalar-as-vector conversion failed for "
                                  << dbgkey_for_errors << ": " << e.what();
      throw std::runtime_error(msg.str());
    }
  }
  std::ostringstream msg; msg << "[RGAFID] Expected sequence (or scalar) for " << dbgkey_for_errors;
  throw std::runtime_error(msg.str());
}

// ---- Config loader (yaml-cpp only)
void RGAFiducialFilter::LoadConfigFromYAML() {
  // Optional: keep this if other Iguana machinery expects it, but we won't use GetOptionVector.
  ParseYAMLConfig();

  // --- Calorimeter (strictness is a SCALAR)
  {
    m_cal_strictness = RGAFID_ReadScalar<int>({"calorimeter","strictness"},
                                              "calorimeter.strictness");
    if (m_cal_strictness < 1 || m_cal_strictness > 3)
      throw std::runtime_error("[RGAFID] 'calorimeter.strictness' must be 1,2,3");
  }

  // --- Forward Tagger (LISTS)
  {
    auto radius = RGAFID_ReadVector<double>({"forward_tagger","radius"},
                                            "forward_tagger.radius");
    if (radius.size() != 2)
      throw std::runtime_error("[RGAFID] 'forward_tagger.radius' must be [rmin,rmax]");
    u_ft_params.rmin = static_cast<float>(radius[0]);
    u_ft_params.rmax = static_cast<float>(radius[1]);
    if (!(std::isfinite(u_ft_params.rmin) && std::isfinite(u_ft_params.rmax)) ||
        !(u_ft_params.rmin > 0.f && u_ft_params.rmax > u_ft_params.rmin))
      throw std::runtime_error("[RGAFID] invalid forward_tagger.radius values");

    auto holes_flat = RGAFID_ReadVector<double>({"forward_tagger","holes_flat"},
                                                "forward_tagger.holes_flat");
    if (!holes_flat.empty() && (holes_flat.size() % 3) != 0)
      throw std::runtime_error("[RGAFID] 'forward_tagger.holes_flat' must have 3N values");
    u_ft_params.holes.clear();
    u_ft_params.holes.reserve(holes_flat.size()/3);
    for (std::size_t i=0;i+2<holes_flat.size();i+=3) {
      float R  = static_cast<float>(holes_flat[i+0]);
      float cx = static_cast<float>(holes_flat[i+1]);
      float cy = static_cast<float>(holes_flat[i+2]);
      if (!(std::isfinite(R) && std::isfinite(cx) && std::isfinite(cy)) || R<=0.f)
        throw std::runtime_error("[RGAFID] invalid FT hole triple in 'holes_flat'");
      u_ft_params.holes.push_back({R,cx,cy});
    }
  }

  // --- CVT (edge_layers LIST; edge_min SCALAR; phi_forbidden LIST)
  {
    m_cvt.edge_layers = RGAFID_ReadVector<int>({"cvt","edge_layers"}, "cvt.edge_layers");
    if (m_cvt.edge_layers.empty())
      throw std::runtime_error("[RGAFID] 'cvt.edge_layers' must be non-empty");

    m_cvt.edge_min = RGAFID_ReadScalar<double>({"cvt","edge_min"}, "cvt.edge_min");

    m_cvt.phi_forbidden_deg = RGAFID_ReadVector<double>({"cvt","phi_forbidden_deg"},
                                                        "cvt.phi_forbidden_deg");
    if (!m_cvt.phi_forbidden_deg.empty() &&
        (m_cvt.phi_forbidden_deg.size() % 2) != 0)
      throw std::runtime_error("[RGAFID] 'cvt.phi_forbidden_deg' must have pairs (2N values)");
  }

  // --- DC (theta_small_deg SCALAR; thresholds triplets LISTS)
  {
    m_dc.theta_small_deg = RGAFID_ReadScalar<double>({"dc","theta_small_deg"},
                                                     "dc.theta_small_deg");

    auto need3 = [&](const char* key) -> std::array<double,3> {
      auto vv = RGAFID_ReadVector<double>({"dc", key}, (std::string("dc.") + key).c_str());
      if (vv.size() != 3) {
        std::ostringstream msg; msg << "[RGAFID] 'dc." << key << "' must be [e1,e2,e3]";
        throw std::runtime_error(msg.str());
      }
      return {vv[0], vv[1], vv[2]};
    };

    auto out  = need3("thresholds_out");
    auto in_s = need3("thresholds_in_smallTheta");
    auto in_l = need3("thresholds_in_largeTheta");

    m_dc.out_e1 = out[0];  m_dc.out_e2 = out[1];  m_dc.out_e3 = out[2];
    m_dc.in_small_e1 = in_s[0]; m_dc.in_small_e2 = in_s[1]; m_dc.in_small_e3 = in_s[2];
    m_dc.in_large_e1 = in_l[0]; m_dc.in_large_e2 = in_l[1]; m_dc.in_large_e3 = in_l[2];
  }
}

void RGAFiducialFilter::Start(hipo::banklist& banks)
{
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

  // Load configuration (scalar+vector logic above)
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
    if (it == edge_at_layer.end()) continue; // missing layer -> pass
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
    if (traj.getInt("detector", i) != 6)    continue; // DC == 6
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


} // namespace iguana::clas12