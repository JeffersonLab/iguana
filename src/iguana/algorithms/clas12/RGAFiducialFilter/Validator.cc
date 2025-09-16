#include "Validator.h"
#include "iguana/services/YAMLReader.h"  // for YAMLReader::node_path_t

#include <TCanvas.h>
#include <TLegend.h>
#include <TEllipse.h>
#include <TStyle.h>
#include <TString.h>
#include <TPad.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace iguana::clas12 {

REGISTER_IGUANA_VALIDATOR(RGAFiducialFilterValidator);

// small util
static bool banklist_has(hipo::banklist& banks, const char* name) {
  for (auto& b : banks) if (b.getSchema().getName() == name) return true;
  return false;
}

// helper to build YAML node paths
using NodePath = iguana::YAMLReader::node_path_t;
static inline NodePath Path(std::initializer_list<const char*> keys) {
  NodePath p;
  for (auto* k : keys) p.emplace_back(std::string(k));
  return p;
}

// ---- Config loader (from Algorithm/ConfigFileReader helpers, not yaml-cpp)
void RGAFiducialFilterValidator::LoadConfigFromYAML() {
  // Always parse; the reader caches internally.
  ParseYAMLConfig();

  // Alternate absolute root we’ll try if the file is rooted at the algorithm name
  constexpr const char* ALT_TOP = "clas12::RGAFiducialFilter";

  // Little helpers: try path P1 first; on failure, retry P2 (absolute) if provided.
  auto getD = [&](const char* dbg, const NodePath& p1, const NodePath& p2 = NodePath{})
                  -> std::vector<double> {
    try { return GetOptionVector<double>(dbg, p1); }
    catch (...) {
      if (!p2.empty()) return GetOptionVector<double>(dbg, p2);
      throw;
    }
  };
  auto getI = [&](const char* dbg, const NodePath& p1, const NodePath& p2 = NodePath{})
                  -> std::vector<int> {
    try { return GetOptionVector<int>(dbg, p1); }
    catch (...) {
      if (!p2.empty()) return GetOptionVector<int>(dbg, p2);
      throw;
    }
  };

  // ---------- PCal strictness (for plotting split only)
  {
    auto v = getI("rgafid.cal.strictness",
                  Path({"calorimeter","strictness"}),
                  Path({ALT_TOP,"calorimeter","strictness"}));
    if (v.empty())
      throw std::runtime_error("[RGAFID][VAL] Missing 'calorimeter.strictness'");
    m_cal_strictness = v.at(0);
    if (m_cal_strictness < 1 || m_cal_strictness > 3)
      throw std::runtime_error("[RGAFID][VAL] 'calorimeter.strictness' must be 1,2,3");
  }

  // ---------- FT overlays + pass logic
  {
    auto radius = getD("rgafid.ft.radius",
                       Path({"forward_tagger","radius"}),
                       Path({ALT_TOP,"forward_tagger","radius"}));
    if (radius.size() != 2)
      throw std::runtime_error("[RGAFID][VAL] 'forward_tagger.radius' must be [rmin,rmax]");
    m_ftdraw.rmin = static_cast<float>(radius[0]);
    m_ftdraw.rmax = static_cast<float>(radius[1]);
    if (!(std::isfinite(m_ftdraw.rmin) && std::isfinite(m_ftdraw.rmax)) ||
        !(m_ftdraw.rmin > 0.f && m_ftdraw.rmax > m_ftdraw.rmin))
      throw std::runtime_error("[RGAFID][VAL] invalid forward_tagger.radius values");

    auto holes_flat = getD("rgafid.ft.holes_flat",
                           Path({"forward_tagger","holes_flat"}),
                           Path({ALT_TOP,"forward_tagger","holes_flat"}));
    if (holes_flat.empty() || (holes_flat.size() % 3) != 0)
      throw std::runtime_error("[RGAFID][VAL] 'forward_tagger.holes_flat' must have 3N values");
    m_ftdraw.holes.clear();
    m_ftdraw.holes.reserve(holes_flat.size() / 3);
    for (std::size_t i = 0; i < holes_flat.size(); i += 3) {
      float R  = static_cast<float>(holes_flat[i+0]);
      float cx = static_cast<float>(holes_flat[i+1]);
      float cy = static_cast<float>(holes_flat[i+2]);
      if (!(std::isfinite(R) && std::isfinite(cx) && std::isfinite(cy)) || R <= 0.f)
        throw std::runtime_error("[RGAFID][VAL] invalid FT hole triple in 'holes_flat'");
      m_ftdraw.holes.push_back({R, cx, cy});
    }
  }

  // ---------- CVT parameters
  {
    m_cvt_params.edge_layers =
      getI("rgafid.cvt.edge_layers",
           Path({"cvt","edge_layers"}),
           Path({ALT_TOP,"cvt","edge_layers"}));
    if (m_cvt_params.edge_layers.empty())
      throw std::runtime_error("[RGAFID][VAL] 'cvt.edge_layers' must be non-empty");

    auto v_edge_min = getD("rgafid.cvt.edge_min",
                           Path({"cvt","edge_min"}),
                           Path({ALT_TOP,"cvt","edge_min"}));
    if (v_edge_min.empty())
      throw std::runtime_error("[RGAFID][VAL] 'cvt.edge_min' must be provided as [value]");
    m_cvt_params.edge_min = v_edge_min.at(0);

    m_cvt_params.phi_forbidden_deg =
      getD("rgafid.cvt.phi_forbidden_deg",
           Path({"cvt","phi_forbidden_deg"}),
           Path({ALT_TOP,"cvt","phi_forbidden_deg"}));
    if (!m_cvt_params.phi_forbidden_deg.empty() &&
        (m_cvt_params.phi_forbidden_deg.size() % 2) != 0)
      throw std::runtime_error("[RGAFID][VAL] 'cvt.phi_forbidden_deg' must have pairs (2N values)");
  }

  // ---------- DC parameters
  {
    auto v_theta_small = getD("rgafid.dc.theta_small_deg",
                              Path({"dc","theta_small_deg"}),
                              Path({ALT_TOP,"dc","theta_small_deg"}));
    if (v_theta_small.empty())
      throw std::runtime_error("[RGAFID][VAL] 'dc.theta_small_deg' must be provided as [value]");
    m_dc_params.theta_small_deg = v_theta_small.at(0);

    auto need3 = [&](const char* key) -> std::array<double,3> {
      auto vv = getD((std::string("rgafid.dc.")+key).c_str(),
                     Path({"dc",key}), Path({ALT_TOP,"dc",key}));
      if (vv.size() != 3) {
        std::ostringstream msg; msg << "[RGAFID][VAL] 'dc." << key << "' must be [e1,e2,e3]";
        throw std::runtime_error(msg.str());
      }
      return {vv[0], vv[1], vv[2]};
    };

    auto out  = need3("thresholds_out");
    auto in_s = need3("thresholds_in_smallTheta");
    auto in_l = need3("thresholds_in_largeTheta");

    m_dc_params.out_e1 = out[0];  m_dc_params.out_e2 = out[1];  m_dc_params.out_e3 = out[2];
    m_dc_params.in_small_e1 = in_s[0]; m_dc_params.in_small_e2 = in_s[1]; m_dc_params.in_small_e3 = in_s[2];
    m_dc_params.in_large_e1 = in_l[0]; m_dc_params.in_large_e2 = in_l[1]; m_dc_params.in_large_e3 = in_l[2];
  }
}

// ---- Booking
void RGAFiducialFilterValidator::BookIfNeeded() {
  // PCal: range 0-45 cm, 4.5 cm bins (width of single bar)
  const int nb = 10;
  const double lo = 0.0, hi = 45.0;

  for (int pid : kPIDs) {
    auto& P = m_cal[pid];
    for (int s=1; s<=6; ++s) {
      if (!P[s].lv_kept) {
        P[s].lv_kept = new TH1D(Form("h_pcal_lv_kept_pid%d_s%d", pid, s),
                                Form("PID %d S%d;length (cm);counts", pid, s), nb, lo, hi);
        P[s].lv_kept->SetStats(0);
      }
      if (!P[s].lv_cut) {
        P[s].lv_cut = new TH1D(Form("h_pcal_lv_cut_pid%d_s%d", pid, s),
                               Form("PID %d S%d;length (cm);counts", pid, s), nb, lo, hi);
        P[s].lv_cut->SetStats(0);
      }
      if (!P[s].lw_kept) {
        P[s].lw_kept = new TH1D(Form("h_pcal_lw_kept_pid%d_s%d", pid, s),
                                Form("PID %d S%d;length (cm);counts", pid, s), nb, lo, hi);
        P[s].lw_kept->SetStats(0);
      }
      if (!P[s].lw_cut) {
        P[s].lw_cut = new TH1D(Form("h_pcal_lw_cut_pid%d_s%d", pid, s),
                               Form("PID %d S%d;length (cm);counts", pid, s), nb, lo, hi);
        P[s].lw_cut->SetStats(0);
      }
    }
  }

  // FT: range for x,y
  for (int pid : kPIDs) {
    auto& F = m_ft_h[pid];
    if (!F.before)
      F.before = new TH2F(Form("h_ft_before_pid%d", pid),
                          Form("FT x-y before (PID %d);x (cm);y (cm)", pid),
                          120, -20, 20, 120, -20, 20);
    if (!F.after)
      F.after  = new TH2F(Form("h_ft_after_pid%d", pid),
                          Form("FT x-y after (PID %d);x (cm);y (cm)", pid),
                          120, -20, 20, 120, -20, 20);
    F.before->SetStats(0);
    F.after->SetStats(0);
  }

  // CVT layer 12: combined hadrons, phi (x) vs theta (y)
  if (!m_cvt_before)
    m_cvt_before = new TH2F("h_cvt_l12_phi_theta_before_all",
      "CVT layer 12 before (hadrons: #pm211,#pm321,#pm2212);phi (deg);theta (deg)",
      180, 0, 360,  90, 0, 90);
  if (!m_cvt_after)
    m_cvt_after  = new TH2F("h_cvt_l12_phi_theta_after_all",
      "CVT layer 12 after (hadrons: #pm211,#pm321,#pm2212);phi (deg);theta (deg)",
      180, 0, 360,  90, 0, 90);
  m_cvt_before->SetStats(0);
  m_cvt_after->SetStats(0);

  // DC edge distributions (range 0-30 cm, 0.3 cm bins)
  auto mk = [](const char* name, const char* title){
    auto* h = new TH1D(name, title, 100, 0.0, 30.0);
    h->SetStats(0);
    return h;
  };
  if (!m_dc_pos.r1_before) {
    m_dc_pos.r1_before = mk("h_dc_pos_r1_before", "DC R1 before (+);edge (cm);counts");
    m_dc_pos.r2_before = mk("h_dc_pos_r2_before", "DC R2 before (+);edge (cm);counts");
    m_dc_pos.r3_before = mk("h_dc_pos_r3_before", "DC R3 before (+);edge (cm);counts");
    m_dc_pos.r1_after  = mk("h_dc_pos_r1_after",  "DC R1 after (+);edge (cm);counts");
    m_dc_pos.r2_after  = mk("h_dc_pos_r2_after",  "DC R2 after (+);edge (cm);counts");
    m_dc_pos.r3_after  = mk("h_dc_pos_r3_after",  "DC R3 after (+);edge (cm);counts");
  }
  if (!m_dc_neg.r1_before) {
    m_dc_neg.r1_before = mk("h_dc_neg_r1_before", "DC R1 before (-);edge (cm);counts");
    m_dc_neg.r2_before = mk("h_dc_neg_r2_before", "DC R2 before (-);edge (cm);counts");
    m_dc_neg.r3_before = mk("h_dc_neg_r3_before", "DC R3 before (-);edge (cm);counts");
    m_dc_neg.r1_after  = mk("h_dc_neg_r1_after",  "DC R1 after (-);edge (cm);counts");
    m_dc_neg.r2_after  = mk("h_dc_neg_r2_after",  "DC R2 after (-);edge (cm);counts");
    m_dc_neg.r3_after  = mk("h_dc_neg_r3_after",  "DC R3 after (-);edge (cm);counts");
  }
}

void RGAFiducialFilterValidator::Start(hipo::banklist& banks) {
  // Banks
  b_particle = GetBankIndex(banks, "REC::Particle");
  if (banklist_has(banks, "REC::Calorimeter")) {
    b_calor = GetBankIndex(banks, "REC::Calorimeter"); m_have_calor=true;
  }
  if (banklist_has(banks, "REC::ForwardTagger")) {
    b_ft = GetBankIndex(banks, "REC::ForwardTagger"); m_have_ft=true;
  }
  if (banklist_has(banks, "REC::Traj")) {
    b_traj = GetBankIndex(banks, "REC::Traj"); m_have_traj=true;
  } else {
    m_have_traj=false;
    m_log->Info("[RGAFID][VAL] REC::Traj not provided; CVT/DC plots disabled. "
                "Re-run with -b REC::Traj to enable trajectory-based plots.");
  }
  b_config = GetBankIndex(banks, "RUN::config");

  // Load overlays and cut parameters from config
  LoadConfigFromYAML();

  // Output
  if (auto dir = GetOutputDirectory()) {
    m_base.Form("%s/rga_fiducial", dir->c_str());
    m_out  = new TFile(Form("%s.root", m_base.Data()), "RECREATE");
  } else {
    m_base = "rga_fiducial";
    m_out  = nullptr;
  }

  BookIfNeeded();
}

// ---- Local helpers (pass/fail tests)
static bool PassCalStrictnessForPIndex(const hipo::bank& cal, int pidx, int strictness) {
  float min_lv = std::numeric_limits<float>::infinity();
  float min_lw = std::numeric_limits<float>::infinity();

  const int n = cal.getRows();
  bool saw = false;
  for (int i=0;i<n;++i) {
    if (cal.getInt("pindex", i) != pidx) continue;
    if (cal.getInt("layer",  i) != 1   ) continue; // PCal only
    saw = true;
    float lv = cal.getFloat("lv", i);
    float lw = cal.getFloat("lw", i);
    if (lv < min_lv) min_lv = lv;
    if (lw < min_lw) min_lw = lw;
  }
  if (!saw) return true; // no PCal association -> pass

  switch (strictness) {
    case 1: return !(min_lw <  9.0f || min_lv <  9.0f);
    case 2: return !(min_lw < 13.5f || min_lv < 13.5f);
    case 3: return !(min_lw < 18.0f || min_lv < 18.0f);
  }
  return true;
}

static bool PassFTForPIndex(const hipo::bank& ft, int pidx,
    float rmin, float rmax, const std::vector<std::array<float,3>>& holes) {
  const int n = ft.getRows();
  for (int i=0;i<n;++i) {
    if (ft.getInt("pindex", i) != pidx) continue;
    const double x = ft.getFloat("x", i);
    const double y = ft.getFloat("y", i);
    const double r = std::sqrt(x*x + y*y);
    if (r < rmin) return false;
    if (r > rmax) return false;
    for (auto const& h : holes) {
      const double d = std::sqrt((x-h[1])*(x-h[1]) + (y-h[2])*(y-h[2]));
      if (d < h[0]) return false;
    }
    return true; // first row decides
  }
  return true; // no FT association -> pass
}

static bool PassCVTForPIndex(const hipo::bank& traj, int pidx,
    const std::vector<int>& edge_layers, double edge_min,
    const std::vector<double>& phi_forbidden_deg) {
  const int n = traj.getRows();
  std::map<int,double> edge_at_layer;
  double x12=0.0, y12=0.0; bool saw12=false;

  for (int i=0; i<n; ++i) {
    if (traj.getInt("pindex", i) != pidx) continue;
    if (traj.getInt("detector", i) != 5)  continue;
    int layer = traj.getInt("layer", i);
    double e = traj.getFloat("edge", i);

    if (std::find(edge_layers.begin(), edge_layers.end(), layer) != edge_layers.end())
      edge_at_layer[layer] = e;

    if (layer==12) { x12 = traj.getFloat("x", i); y12 = traj.getFloat("y", i); saw12=true; }
  }

  for (int L : edge_layers) {
    auto it = edge_at_layer.find(L);
    if (it==edge_at_layer.end()) continue; // missing layer -> pass
    if (!(it->second > edge_min)) return false;
  }

  if (saw12 && !phi_forbidden_deg.empty()) {
    double phi = std::atan2(y12, x12)*(180.0/M_PI);
    if (phi<0) phi += 360.0;
    for (std::size_t i=0;i+1<phi_forbidden_deg.size();i+=2) {
      double lo=phi_forbidden_deg[i], hi=phi_forbidden_deg[i+1];
      if (phi>lo && phi<hi) return false;
    }
  }
  return true;
}

static bool PassDCForPIndex(const hipo::bank& particle, const hipo::bank& config,
    const hipo::bank& traj, int pidx, double theta_small_deg,
    double in_small_e1, double in_small_e2, double in_small_e3,
    double in_large_e1, double in_large_e2, double in_large_e3,
    double out_e1, double out_e2, double out_e3) {
  const int pid = particle.getInt("pid", pidx);
  const bool isNeg = (pid== 11 || pid==-211 || pid==-321 || pid==-2212);
  const bool isPos = (pid==-11 || pid== 211 || pid== 321 || pid== 2212);
  if (!(isNeg || isPos)) return false;

  const float torus = config.getFloat("torus", 0);
  const bool electron_out = (torus == 1.0f);
  const bool particle_inb = (electron_out ? isPos : isNeg);
  const bool particle_out = !particle_inb;

  const double px = particle.getFloat("px", pidx);
  const double py = particle.getFloat("py", pidx);
  const double pz = particle.getFloat("pz", pidx);
  const double rho = std::sqrt(px*px + py*py);
  const double theta = std::atan2(rho, (pz==0.0 ? 1e-9 : pz)) * (180.0/M_PI);

  double e1=0.0, e2=0.0, e3=0.0;
  const int n = traj.getRows();
  for (int i=0;i<n;++i) {
    if (traj.getInt("pindex", i) != pidx) continue;
    if (traj.getInt("detector", i) != 6)  continue;
    int layer = traj.getInt("layer", i);
    double e  = traj.getFloat("edge", i);
    if      (layer== 6) e1 = e;
    else if (layer==18) e2 = e;
    else if (layer==36) e3 = e;
  }

  if (particle_inb) {
    if (theta < theta_small_deg) return (e1>in_small_e1 && e2>in_small_e2 && e3>in_small_e3);
    return (e1>in_large_e1 && e2>in_large_e2 && e3>in_large_e3);
  } else if (particle_out) {
    return (e1>out_e1 && e2>out_e2 && e3>out_e3);
  }
  return false;
}

// Free function (not a class method) to set DC pad margins.
static inline void SetDCPadMargins() {
  gPad->SetLeftMargin(0.16);
  gPad->SetRightMargin(0.06);
  gPad->SetBottomMargin(0.12);
  gPad->SetTopMargin(0.08);
}

// ---- Full Run() including PCal
void RGAFiducialFilterValidator::Run(hipo::banklist& banks) const {
  auto& particle = GetBank(banks, b_particle, "REC::Particle");
  auto& config   = GetBank(banks, b_config,   "RUN::config");

  // Track torus polarity stats (for DC labels)
  {
    bool e_out = (config.getFloat("torus", 0) == 1.0f);
    if (e_out) const_cast<RGAFiducialFilterValidator*>(this)->m_torus_out_events++;
    else       const_cast<RGAFiducialFilterValidator*>(this)->m_torus_in_events++;
  }

  // Snapshot: pindex sets
  std::unordered_set<int> electrons_or_photons;
  std::unordered_set<int> hadrons;
  std::unordered_set<int> pos_all, neg_all;

  for (auto const& row : particle.getRowList()) {
    int pidx = (int)row;
    int pid  = particle.getInt("pid", row);
    if (pid==11 || pid==22) electrons_or_photons.insert(pidx);
    if (pid==211 || pid==321 || pid==2212 || pid==-211 || pid==-321 || pid==-2212)
      hadrons.insert(pidx);
    if (pid>0) pos_all.insert(pidx);
    else if (pid<0) neg_all.insert(pidx);
  }

  // PCal kept vs cut (electrons/photons), strictness from YAML
  if (m_have_calor) {
    auto& cal = GetBank(banks, b_calor, "REC::Calorimeter");
    const int n = cal.getRows();

    std::unordered_map<int, bool> pass_cache;
    for (int pidx : electrons_or_photons)
      pass_cache[pidx] = PassCalStrictnessForPIndex(cal, pidx, m_cal_strictness);

    std::array<std::set<int>,7> b_e, a_e, b_g, a_g;

    for (int i=0;i<n;++i) {
      int pidx = cal.getInt("pindex", i);
      if (!electrons_or_photons.count(pidx)) continue;
      if (cal.getInt("layer", i) != 1) continue;

      int pid = 11;
      for (auto r : particle.getRowList()) { if ((int)r == pidx) {
        pid = particle.getInt("pid", r); break; } }

      int sec = cal.getInt("sector", i);
      if (sec < 1 || sec > 6) continue;

      double lv = cal.getFloat("lv", i);
      double lw = cal.getFloat("lw", i);
      bool kept = pass_cache[pidx];

      auto& H = const_cast<RGAFiducialFilterValidator*>(this)->m_cal[pid][sec];
      if (lv >= 0.0 && lv <= 45.0) (kept ? H.lv_kept : H.lv_cut)->Fill(lv);
      if (lw >= 0.0 && lw <= 45.0) (kept ? H.lw_kept : H.lw_cut)->Fill(lw);

      if (pid==11) {
        if (!b_e[sec].count(pidx)) b_e[sec].insert(pidx);
        if (kept && !a_e[sec].count(pidx)) a_e[sec].insert(pidx);
      } else {
        if (!b_g[sec].count(pidx)) b_g[sec].insert(pidx);
        if (kept && !a_g[sec].count(pidx)) a_g[sec].insert(pidx);
      }
    }

    for (int s=1;s<=6;++s) {
      const_cast<RGAFiducialFilterValidator*>(this)->m_cal_counts[11][s].before += b_e[s].size();
      const_cast<RGAFiducialFilterValidator*>(this)->m_cal_counts[11][s].after  += a_e[s].size();
      const_cast<RGAFiducialFilterValidator*>(this)->m_cal_counts[22][s].before += b_g[s].size();
      const_cast<RGAFiducialFilterValidator*>(this)->m_cal_counts[22][s].after  += a_g[s].size();
    }
  }

  // FT before/after (e-/gamma)
  if (m_have_ft) {
    auto& ft = GetBank(banks, b_ft, "REC::ForwardTagger");

    std::set<int> seen_b_e, seen_a_e, seen_b_g, seen_a_g;
    std::unordered_map<int,bool> pass_cache;

    for (int pidx : electrons_or_photons)
      pass_cache[pidx] = PassFTForPIndex(ft, pidx, m_ftdraw.rmin, m_ftdraw.rmax, m_ftdraw.holes);

    const int n = ft.getRows();
    for (int i=0;i<n;++i) {
      int pidx = ft.getInt("pindex", i);
      if (!electrons_or_photons.count(pidx)) continue;

      int pid = 11;
      for (auto r : particle.getRowList()) if ((int)r==pidx) {
        pid = particle.getInt("pid", r); break;
      }

      auto& HH = const_cast<RGAFiducialFilterValidator*>(this)->m_ft_h.at(pid);

      if (pid==11) {
        if (!seen_b_e.count(pidx)) {
          HH.before->Fill(ft.getFloat("x", i), ft.getFloat("y", i)); seen_b_e.insert(pidx); }
        if ( pass_cache[pidx] && !seen_a_e.count(pidx)) {
          HH.after ->Fill(ft.getFloat("x", i), ft.getFloat("y", i)); seen_a_e.insert(pidx); }
      } else {
        if (!seen_b_g.count(pidx)) {
          HH.before->Fill(ft.getFloat("x", i), ft.getFloat("y", i)); seen_b_g.insert(pidx); }
        if ( pass_cache[pidx] && !seen_a_g.count(pidx)) {
          HH.after ->Fill(ft.getFloat("x", i), ft.getFloat("y", i)); seen_a_g.insert(pidx); }
      }
    }

    const_cast<RGAFiducialFilterValidator*>(this)->m_ft_before_n[11] += seen_b_e.size();
    const_cast<RGAFiducialFilterValidator*>(this)->m_ft_after_n [11] += seen_a_e.size();
    const_cast<RGAFiducialFilterValidator*>(this)->m_ft_before_n[22] += seen_b_g.size();
    const_cast<RGAFiducialFilterValidator*>(this)->m_ft_after_n [22] += seen_a_g.size();
  }

  // CVT layer12 phi/theta before/after (hadrons)
  if (m_have_traj) {
    auto& traj = GetBank(banks, b_traj, "REC::Traj");

    std::set<int> b_seen, a_seen;

    std::unordered_map<int,bool> pass_cache;
    for (int pidx : hadrons)
      pass_cache[pidx] = PassCVTForPIndex(traj, pidx, m_cvt_params.edge_layers,
        m_cvt_params.edge_min, m_cvt_params.phi_forbidden_deg);

    const int n = traj.getRows();
    for (int i=0; i<n; ++i) {
      if (traj.getInt("detector", i) != 5) continue;
      if (traj.getInt("layer", i) != 12) continue;

      int pidx = traj.getInt("pindex", i);
      if (!hadrons.count(pidx)) continue;

      double x = traj.getFloat("x", i);
      double y = traj.getFloat("y", i);
      double z = traj.getFloat("z", i);

      double phi = std::atan2(y, x) * (180.0/M_PI); if (phi < 0) phi += 360.0;
      double rho = std::sqrt(x*x + y*y);
      double theta = std::atan2(rho, (z==0.0 ? 1e-9 : z)) * (180.0/M_PI);

      if (!b_seen.count(pidx)) { m_cvt_before->Fill(phi, theta); b_seen.insert(pidx); }
      if ( pass_cache[pidx] && !a_seen.count(pidx)) {
        m_cvt_after->Fill(phi, theta); a_seen.insert(pidx);
      }
    }

    const_cast<RGAFiducialFilterValidator*>(this)->m_cvt_before_n += (long long) b_seen.size();
    const_cast<RGAFiducialFilterValidator*>(this)->m_cvt_after_n  += (long long) a_seen.size();
  }

  // DC edges pos/neg before/after
  if (m_have_traj) {
    auto& traj = GetBank(banks, b_traj, "REC::Traj");

    std::set<int> pos_b1, pos_b2, pos_b3, neg_b1, neg_b2, neg_b3;
    std::set<int> pos_a1, pos_a2, pos_a3, neg_a1, neg_a2, neg_a3;

    std::unordered_map<int,bool> pass_cache;
    for (int pidx : pos_all)
      pass_cache[pidx] = PassDCForPIndex(particle, config, traj, pidx,
        m_dc_params.theta_small_deg, m_dc_params.in_small_e1,
        m_dc_params.in_small_e2, m_dc_params.in_small_e3,
        m_dc_params.in_large_e1, m_dc_params.in_large_e2, m_dc_params.in_large_e3,
        m_dc_params.out_e1, m_dc_params.out_e2, m_dc_params.out_e3);
    for (int pidx : neg_all)
      pass_cache[pidx] = PassDCForPIndex(particle, config, traj, pidx,
        m_dc_params.theta_small_deg, m_dc_params.in_small_e1,
        m_dc_params.in_small_e2, m_dc_params.in_small_e3,
        m_dc_params.in_large_e1, m_dc_params.in_large_e2, m_dc_params.in_large_e3,
        m_dc_params.out_e1, m_dc_params.out_e2, m_dc_params.out_e3);

    const int n = traj.getRows();
    for (int i=0;i<n;++i) {
      if (traj.getInt("detector", i) != 6) continue;

      int pidx = traj.getInt("pindex", i);
      int pid  = 0;
      for (auto r : particle.getRowList()) if ((int)r==pidx) {
        pid = particle.getInt("pid", r); break;
      }
      if (pid==0 || pid==22) continue;

      double edge = traj.getFloat("edge", i);
      int layer   = traj.getInt("layer", i);

      if (pid>0) {
        if (layer==6)  { if (!pos_b1.count(pidx)) {
            m_dc_pos.r1_before->Fill(edge); pos_b1.insert(pidx);
          }
          if (pass_cache[pidx] && !pos_a1.count(pidx)) {
            m_dc_pos.r1_after->Fill(edge);  pos_a1.insert(pidx); }
        }
        if (layer==18) { if (!pos_b2.count(pidx)) {
            m_dc_pos.r2_before->Fill(edge); pos_b2.insert(pidx);
          }
          if (pass_cache[pidx] && !pos_a2.count(pidx)) {
            m_dc_pos.r2_after->Fill(edge);  pos_a2.insert(pidx); }
        }
        if (layer==36) { if (!pos_b3.count(pidx)) {
            m_dc_pos.r3_before->Fill(edge); pos_b3.insert(pidx);
          }
          if (pass_cache[pidx] && !pos_a3.count(pidx)) {
            m_dc_pos.r3_after->Fill(edge);  pos_a3.insert(pidx);
          }
        }
      } else {
        if (layer==6)  { if (!neg_b1.count(pidx)) {
            m_dc_neg.r1_before->Fill(edge); neg_b1.insert(pidx);
          }
          if (pass_cache[pidx] && !neg_a1.count(pidx)) {
            m_dc_neg.r1_after->Fill(edge);  neg_a1.insert(pidx);
          }
        }
        if (layer==18) { if (!neg_b2.count(pidx)) {
            m_dc_neg.r2_before->Fill(edge); neg_b2.insert(pidx);
          }
          if (pass_cache[pidx] && !neg_a2.count(pidx)) {
            m_dc_neg.r2_after->Fill(edge);  neg_a2.insert(pidx);
          }
        }
        if (layer==36) { if (!neg_b3.count(pidx)) {
            m_dc_neg.r3_before->Fill(edge); neg_b3.insert(pidx);
          }
          if (pass_cache[pidx] && !neg_a3.count(pidx)) {
            m_dc_neg.r3_after->Fill(edge);  neg_a3.insert(pidx);
          }
        }
      }
    }

    // helper: size
    auto inter3 = [](const std::set<int>& A,
        const std::set<int>& B,
        const std::set<int>& C)->size_t {
      const std::set<int>* smallest = &A;
      if (B.size() < smallest->size()) smallest = &B;
      if (C.size() < smallest->size()) smallest = &C;
      size_t cnt=0;
      for (int v : *smallest) if (A.count(v) && B.count(v) && C.count(v)) ++cnt;
      return cnt;
    };

    // accumulate the totals using the intersection across regions
    const_cast<RGAFiducialFilterValidator*>(this)->m_dc_pos_before_n +=
      (long long) inter3(pos_b1, pos_b2, pos_b3);
    const_cast<RGAFiducialFilterValidator*>(this)->m_dc_pos_after_n  +=
      (long long) inter3(pos_a1, pos_a2, pos_a3);
    const_cast<RGAFiducialFilterValidator*>(this)->m_dc_neg_before_n +=
      (long long) inter3(neg_b1, neg_b2, neg_b3);
    const_cast<RGAFiducialFilterValidator*>(this)->m_dc_neg_after_n  +=
      (long long) inter3(neg_a1, neg_a2, neg_a3);
  }
}

// ---- Drawing helpers
void RGAFiducialFilterValidator::DrawCalCanvas(int pid, const char* title) {
  auto it = m_cal.find(pid);
  if (it == m_cal.end()) return;

  auto* c = new TCanvas(Form("rgafid_pcal_pid%d", pid), title, 1400, 900);
  c->Divide(3,2);

  for (int s=1; s<=6; ++s) {
    c->cd(s);
    gPad->SetLeftMargin(0.12); gPad->SetRightMargin(0.04);
    gPad->SetBottomMargin(0.12); gPad->SetTopMargin(0.08);

    auto& H = it->second[s];
    if (!H.lv_kept) continue;

    H.lv_kept->SetLineColor(kBlue+1);  H.lv_kept->SetLineWidth(2);  H.lv_kept->SetLineStyle(1);
    H.lw_kept->SetLineColor(kRed+1);   H.lw_kept->SetLineWidth(2);  H.lw_kept->SetLineStyle(1);
    H.lv_cut ->SetLineColor(kBlue+1);  H.lv_cut ->SetLineWidth(2);  H.lv_cut ->SetLineStyle(2);
    H.lw_cut ->SetLineColor(kRed+1);   H.lw_cut ->SetLineWidth(2);  H.lw_cut ->SetLineStyle(2);

    // survival % for this PID/sector
    long long b = m_cal_counts[pid][s].before;
    long long a = m_cal_counts[pid][s].after;
    double pct = (b>0) ? (100.0*double(a)/double(b)) : 0.0;

    H.lv_kept->SetTitle(Form("%s - Sector %d  [survive = %.3f%%];length (cm);counts",
      pid==11?"Electrons":"Photons", s, pct));

    H.lv_kept->Draw("HIST");
    H.lw_kept->Draw("HISTSAME");
    H.lv_cut ->Draw("HISTSAME");
    H.lw_cut ->Draw("HISTSAME");

    auto* leg = new TLegend(0.55, 0.72, 0.88, 0.90);
    leg->SetBorderSize(0); leg->SetFillStyle(0);
    leg->AddEntry(H.lv_kept, "lv kept", "l");
    leg->AddEntry(H.lw_kept, "lw kept", "l");
    leg->AddEntry(H.lv_cut , "lv cut",  "l");
    leg->AddEntry(H.lw_cut , "lw cut",  "l");
    leg->Draw();
  }

  c->SaveAs(Form("%s_pcal_lv_lw_pid%d.png", m_base.Data(), pid));
}

void RGAFiducialFilterValidator::DrawFTCanvas2x2() {
  if (!m_have_ft) return;

  auto* c = new TCanvas("rgafid_ft_xy_2x2", "FT x-y Before/After", 1200, 900);
  c->Divide(2,2);

  auto draw_pad = [&](int pad, TH2F* h, const char* ttl){
    c->cd(pad);
    gPad->SetLeftMargin(0.12); gPad->SetRightMargin(0.04);
    gPad->SetBottomMargin(0.12); gPad->SetTopMargin(0.08);
    h->SetTitle(ttl);
    h->Draw("COLZ");

    // overlays: annulus + holes
    auto* outer = new TEllipse(0,0, m_ftdraw.rmax, m_ftdraw.rmax);
    auto* inner = new TEllipse(0,0, m_ftdraw.rmin, m_ftdraw.rmin);
    outer->SetFillStyle(0); inner->SetFillStyle(0);
    outer->SetLineStyle(2); inner->SetLineStyle(2);
    outer->Draw(); inner->Draw();
    for (auto const& H : m_ftdraw.holes) {
      auto* e = new TEllipse(H[1], H[2], H[0], H[0]);
      e->SetFillStyle(0); e->SetLineColor(kBlack); e->SetLineStyle(7); e->Draw();
    }
  };

  auto pct = [&](int pid)->double{
    long long b = m_ft_before_n[pid];
    long long a = m_ft_after_n[pid];
    return (b>0) ? (100.0*double(a)/double(b)) : 0.0;
  };

  draw_pad(1, m_ft_h.at(11).before, "Electrons (before cuts);x (cm);y (cm)");
  draw_pad(2, m_ft_h.at(11).after,
    Form("Electrons (after cuts)  [survive = %.3f%%];x (cm);y (cm)", pct(11)));

  draw_pad(3, m_ft_h.at(22).before, "Photons (before cuts);x (cm);y (cm)");
  draw_pad(4, m_ft_h.at(22).after,
    Form("Photons (after cuts)  [survive = %.3f%%];x (cm);y (cm)", pct(22)));

  c->SaveAs(Form("%s_ft_xy_2x2.png", m_base.Data()));
}

void RGAFiducialFilterValidator::DrawCVTCanvas1x2(const char* title) {
  if (!m_have_traj || !m_cvt_before || !m_cvt_after) return;

  auto* c = new TCanvas("rgafid_cvt_l12_all", title, 1200, 600);
  c->Divide(2,1);

  const double left   = 0.12;
  const double right  = 0.16;
  const double bottom = 0.12;
  const double top    = 0.08;

  double pct = 0.0;
  if (m_cvt_before_n > 0) pct = 100.0 * double(m_cvt_after_n) / double(m_cvt_before_n);

  c->cd(1);
  gPad->SetLeftMargin(left);
  gPad->SetRightMargin(right);
  gPad->SetBottomMargin(bottom);
  gPad->SetTopMargin(top);
  m_cvt_before->Draw("COLZ");

  c->cd(2);
  gPad->SetLeftMargin(left);
  gPad->SetRightMargin(right);
  gPad->SetBottomMargin(bottom);
  gPad->SetTopMargin(top);
  m_cvt_after->SetTitle(Form("CVT layer 12 after (hadrons: #pm211,#pm321,#pm2212)  [survive = %.3f%%];phi (deg);theta (deg)", pct));
  m_cvt_after->Draw("COLZ");

  c->SaveAs(Form("%s_cvt_l12_phi_theta_hadrons.png", m_base.Data()));
}

void RGAFiducialFilterValidator::DrawDCCanvas2x3(const DCHists& H,
  const char* bend, double survive_pct) {

  TString bendTitle = (TString(bend)=="inb") ? "Inb" : "Out";

  auto* c = new TCanvas(Form("rgafid_dc_%s_2x3", bend),
    Form("%s DC edges: before/after", bendTitle.Data()), 1500, 900);
  c->Divide(3,2);

  // before row
  c->cd(1); SetDCPadMargins(); if (H.r1_before) {
    H.r1_before->SetLineWidth(2);
    H.r1_before->Draw("HIST");
    H.r1_before->SetTitle(Form("%s DC Region 1 (before);edge (cm);counts", bendTitle.Data())); }
  c->cd(2); SetDCPadMargins(); if (H.r2_before) {
    H.r2_before->SetLineWidth(2);
    H.r2_before->Draw("HIST");
    H.r2_before->SetTitle(Form("%s DC Region 2 (before);edge (cm);counts", bendTitle.Data())); }
  c->cd(3); SetDCPadMargins(); if (H.r3_before) {
    H.r3_before->SetLineWidth(2);
    H.r3_before->Draw("HIST");
    H.r3_before->SetTitle(Form("%s DC Region 3 (before);edge (cm);counts", bendTitle.Data())); }

  // after row with survive %
  c->cd(4); SetDCPadMargins(); if (H.r1_after)  {
    H.r1_after ->SetLineWidth(2);
    H.r1_after ->Draw("HIST");
    H.r1_after ->SetTitle(Form("%s DC Region 1 (after)  [survive = %.3f%%];edge (cm);counts", bendTitle.Data(), survive_pct)); }
  c->cd(5); SetDCPadMargins(); if (H.r2_after)  {
    H.r2_after ->SetLineWidth(2);
    H.r2_after ->Draw("HIST");
    H.r2_after ->SetTitle(Form("%s DC Region 2 (after)  [survive = %.3f%%];edge (cm);counts", bendTitle.Data(), survive_pct)); }
  c->cd(6); SetDCPadMargins(); if (H.r3_after)  {
    H.r3_after ->SetLineWidth(2);
    H.r3_after ->Draw("HIST");
    H.r3_after ->SetTitle(Form("%s DC Region 3 (after)  [survive = %.3f%%];edge (cm);counts", bendTitle.Data(), survive_pct)); }

  c->SaveAs(Form("%s_dc_%s_2x3.png", m_base.Data(), bend));
}

void RGAFiducialFilterValidator::Stop() {
  // PCal canvases
  DrawCalCanvas(11, "PCal lv & lw (Electrons): kept solid, cut dashed");
  DrawCalCanvas(22, "PCal lv & lw (Photons): kept solid, cut dashed");

  // FT 2x2
  DrawFTCanvas2x2();

  // CVT layer 12 1x2 (combined hadrons) with survive %
  DrawCVTCanvas1x2("CVT layer 12 (Hadrons): phi vs theta");

  // Decide torus labels
  bool electron_out = (m_torus_out_events >= m_torus_in_events);
  const char* pos_bend_id = electron_out ? "inb" : "out";
  const char* neg_bend_id = electron_out ? "out" : "inb";

  // DC canvases
  double pos_pct =
    (m_dc_pos_before_n>0) ? (100.0*double(m_dc_pos_after_n)/double(m_dc_pos_before_n)) : 0.0;
  double neg_pct =
    (m_dc_neg_before_n>0) ? (100.0*double(m_dc_neg_after_n)/double(m_dc_neg_before_n)) : 0.0;
  DrawDCCanvas2x3(m_dc_pos, pos_bend_id, pos_pct);
  DrawDCCanvas2x3(m_dc_neg, neg_bend_id, neg_pct);

  if (m_out) {
    m_out->Write();
    m_log->Info("Wrote output file {}", m_out->GetName());
    m_out->Close();
  }
}

} 