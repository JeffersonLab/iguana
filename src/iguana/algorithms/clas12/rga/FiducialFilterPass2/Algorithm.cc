#include "Algorithm.h"
#include "TypeDefs.h"

namespace iguana::clas12::rga {

  REGISTER_IGUANA_ALGORITHM(FiducialFilterPass2);

  static bool banklist_has(hipo::banklist& banks, char const* name)
  {
    for(auto& b : banks)
      if(b.getSchema().getName() == name)
        return true;
    return false;
  }

  static bool traj_has_detector(hipo::bank const* trajBank, int pindex, int detector)
  {
    if(!trajBank)
      return false;
    auto const& traj = *trajBank;
    int const n      = traj.getRows();
    for(int i = 0; i < n; ++i) {
      if(traj.getInt("pindex", i) == pindex && traj.getInt("detector", i) == detector) {
        return true;
      }
    }
    return false;
  }

  void FiducialFilterPass2::ConfigHook()
  {
    m_cal_strictness = GetOptionScalar<int>({"calorimeter", "strictness"});
    if(m_cal_strictness < 1 || m_cal_strictness > 3) {
      throw std::runtime_error("[RGAFID] 'calorimeter.strictness' must be 1, 2, or 3");
    }

    {
      auto radius = GetOptionVector<double>({"forward_tagger", "radius"});
      if(radius.size() != 2) {
        throw std::runtime_error("[RGAFID] 'forward_tagger.radius' must be [rmin, rmax]");
      }
      u_ft_params.rmin = static_cast<float>(radius[0]);
      u_ft_params.rmax = static_cast<float>(radius[1]);
      if(!(std::isfinite(u_ft_params.rmin) && std::isfinite(u_ft_params.rmax)) ||
         !(u_ft_params.rmin > 0.f && u_ft_params.rmax > u_ft_params.rmin)) {
        throw std::runtime_error("[RGAFID] invalid forward_tagger.radius values");
      }

      u_ft_params.holes.clear();
      std::vector<double> holes_flat;
      try {
        holes_flat = GetOptionVector<double>({"forward_tagger", "holes_flat"});
      }
      catch(std::exception const& e) {
        std::string const msg = e.what();
        if(msg.find("not found") == std::string::npos &&
           msg.find("missing") == std::string::npos) {
          throw;
        }
        holes_flat.clear();
      }

      if(!holes_flat.empty() && (holes_flat.size() % 3) != 0) {
        throw std::runtime_error("[RGAFID] 'forward_tagger.holes_flat' must have 3N values");
      }
      u_ft_params.holes.reserve(holes_flat.size() / 3);
      for(std::size_t i = 0; i + 2 < holes_flat.size(); i += 3) {
        float const R  = static_cast<float>(holes_flat[i + 0]);
        float const cx = static_cast<float>(holes_flat[i + 1]);
        float const cy = static_cast<float>(holes_flat[i + 2]);
        if(!(std::isfinite(R) && std::isfinite(cx) && std::isfinite(cy)) || R <= 0.f) {
          throw std::runtime_error("[RGAFID] invalid FT hole triple in 'holes_flat'");
        }
        u_ft_params.holes.push_back({R, cx, cy});
      }
    }

    {
      m_cvt.edge_layers = GetOptionVector<int>({"cvt", "edge_layers"});
      if(m_cvt.edge_layers.empty()) {
        throw std::runtime_error("[RGAFID] 'cvt.edge_layers' must be non-empty");
      }
      m_cvt.edge_min = GetOptionScalar<double>({"cvt", "edge_min"});

      m_cvt.phi_forbidden_deg.clear();
      try {
        m_cvt.phi_forbidden_deg = GetOptionVector<double>({"cvt", "phi_forbidden_deg"});
      }
      catch(std::exception const& e) {
        std::string const msg = e.what();
        if(msg.find("not found") == std::string::npos &&
           msg.find("missing") == std::string::npos) {
          throw;
        }
        m_cvt.phi_forbidden_deg.clear();
      }
      if(!m_cvt.phi_forbidden_deg.empty() && (m_cvt.phi_forbidden_deg.size() % 2) != 0) {
        throw std::runtime_error("[RGAFID] 'cvt.phi_forbidden_deg' must have pairs (2N values)");
      }
    }

    {
      m_dc.theta_small_deg =
          GetOptionScalar<double>({"dc", "theta_small_deg"});

      auto need3 = [&](char const* key) -> std::array<double, 3> {
        auto v = GetOptionVector<double>({"dc", key});
        if(v.size() != 3) {
          throw std::runtime_error(std::string("[RGAFID] 'dc.") + key + "' must be [e1,e2,e3]");
        }
        return {v[0], v[1], v[2]};
      };

      auto out  = need3("thresholds_out");
      auto in_s = need3("thresholds_in_smallTheta");
      auto in_l = need3("thresholds_in_largeTheta");

      m_dc.out_e1      = out[0];
      m_dc.out_e2      = out[1];
      m_dc.out_e3      = out[2];
      m_dc.in_small_e1 = in_s[0];
      m_dc.in_small_e2 = in_s[1];
      m_dc.in_small_e3 = in_s[2];
      m_dc.in_large_e1 = in_l[0];
      m_dc.in_large_e2 = in_l[1];
      m_dc.in_large_e3 = in_l[2];
    }
  }

  void FiducialFilterPass2::StartHook(hipo::banklist& banks)
  {
    b_particle = GetBankIndex(banks, "REC::Particle");
    b_config   = GetBankIndex(banks, "RUN::config");
    if(banklist_has(banks, "REC::Calorimeter")) {
      b_calor      = GetBankIndex(banks, "REC::Calorimeter");
      m_have_calor = true;
    }
    if(banklist_has(banks, "REC::ForwardTagger")) {
      b_ft      = GetBankIndex(banks, "REC::ForwardTagger");
      m_have_ft = true;
    }
    if(banklist_has(banks, "REC::Traj")) {
      b_traj      = GetBankIndex(banks, "REC::Traj");
      m_have_traj = true;
    }
  }

  bool FiducialFilterPass2::RunHook(hipo::banklist& banks) const
  {
    return Run(
        GetBank(banks, b_particle, "REC::Particle"),
        GetBank(banks, b_config, "RUN::config"),
        m_have_calor ? &GetBank(banks, b_calor, "REC::Calorimeter") : nullptr,
        m_have_traj ? &GetBank(banks, b_traj, "REC::Traj") : nullptr,
        m_have_ft ? &GetBank(banks, b_ft, "REC::ForwardTagger") : nullptr);
  }

  bool FiducialFilterPass2::Run(
      hipo::bank& particle,
      hipo::bank const& conf,
      hipo::bank const* cal,
      hipo::bank const* traj,
      hipo::bank const* ft) const
  {
    particle.getMutableRowList().filter([this, &conf, cal, traj, ft](auto bank, auto row) {
      bool const keep = Filter(row, bank, conf, cal, traj, ft);
      return keep ? 1 : 0;
    });
    return !particle.getRowList().empty();
  }

  FiducialFilterPass2::CalLayers
  FiducialFilterPass2::CollectCalHitsForTrack(hipo::bank const& cal, int pindex)
  {
    CalLayers out;
    int const n = cal.getRows();
    for(int i = 0; i < n; ++i) {
      if(cal.getInt("pindex", i) != pindex)
        continue;
      if(cal.getInt("layer", i) != DetectorLayer::PCAL)
        continue; // PCal only
      CalHit h;
      h.sector = cal.getInt("sector", i);
      h.lv     = cal.getFloat("lv", i);
      h.lw     = cal.getFloat("lw", i);
      h.lu     = cal.getFloat("lu", i);
      out.L1.push_back(h);
      out.has_any = true;
    }
    return out;
  }

  bool FiducialFilterPass2::PassCalStrictness(CalLayers const& H, int strictness)
  {
    if(!H.has_any)
      return true;

    float min_lv = std::numeric_limits<float>::infinity();
    float min_lw = std::numeric_limits<float>::infinity();
    for(auto const& hit : H.L1) {
      if(std::isfinite(hit.lv) && hit.lv < min_lv)
        min_lv = hit.lv;
      if(std::isfinite(hit.lw) && hit.lw < min_lw)
        min_lw = hit.lw;
    }

    float const thr = (strictness == 1 ? 9.0f : strictness == 2 ? 13.5f
                                                                : 18.0f);
    return !(min_lv < thr || min_lw < thr);
  }

  bool FiducialFilterPass2::PassFTFiducial(int pindex, hipo::bank const* ftBank) const
  {
    if(!ftBank)
      return true;

    auto const& ft = *ftBank;
    int const n    = ft.getRows();
    for(int i = 0; i < n; ++i) {
      if(ft.getInt("pindex", i) != pindex)
        continue;

      double const x = ft.getFloat("x", i);
      double const y = ft.getFloat("y", i);
      double const r = std::hypot(x, y);

      if(r < u_ft_params.rmin)
        return false;
      if(r > u_ft_params.rmax)
        return false;

      for(auto const& H : u_ft_params.holes) {
        double const R = H[0], cx = H[1], cy = H[2];
        double const d = std::hypot(x - cx, y - cy);
        if(d < R)
          return false;
      }
      return true;
    }
    return true;
  }

  bool FiducialFilterPass2::PassCVTFiducial(int pindex, hipo::bank const* trajBank) const
  {
    if(!trajBank)
      return true;

    auto const& traj = *trajBank;
    int const n      = traj.getRows();

    std::map<int, double> edge_at_layer;

    double x12 = 0.0, y12 = 0.0;
    bool saw12 = false;

    for(int i = 0; i < n; ++i) {
      if(traj.getInt("pindex", i) != pindex)
        continue;
      if(traj.getInt("detector", i) != DetectorType::CVT)
        continue;

      int const layer   = traj.getInt("layer", i);
      double const edge = static_cast<double>(traj.getFloat("edge", i));

      if(std::find(m_cvt.edge_layers.begin(), m_cvt.edge_layers.end(), layer) != m_cvt.edge_layers.end()) {
        edge_at_layer[layer] = edge;
      }

      if(layer == 12) {
        double const x = static_cast<double>(traj.getFloat("x", i));
        double const y = static_cast<double>(traj.getFloat("y", i));
        if(std::isfinite(x) && std::isfinite(y)) {
          x12   = x;
          y12   = y;
          saw12 = true;
        }
      }
    }

    for(int L : m_cvt.edge_layers) {
      auto it = edge_at_layer.find(L);
      if(it == edge_at_layer.end())
        continue;
      if(!(it->second > m_cvt.edge_min)) {
        return false;
      }
    }

    if(saw12 && !m_cvt.phi_forbidden_deg.empty()) {
      double phi = std::atan2(y12, x12) * (180.0 / M_PI);
      if(phi < 0)
        phi += 360.0;
      for(std::size_t i = 0; i + 1 < m_cvt.phi_forbidden_deg.size(); i += 2) {
        double const lo = m_cvt.phi_forbidden_deg[i];
        double const hi = m_cvt.phi_forbidden_deg[i + 1];
        if(phi > lo && phi < hi) {
          return false;
        }
      }
    }

    return true;
  }

  bool FiducialFilterPass2::PassDCFiducial(int pindex, hipo::bank const& particleBank,
                                           hipo::bank const& configBank, hipo::bank const* trajBank) const
  {
    if(!trajBank)
      return true;

    int const pid    = particleBank.getInt("pid", pindex);
    bool const isNeg = (pid == 11 || pid == -211 || pid == -321 || pid == -2212);
    bool const isPos = (pid == -11 || pid == 211 || pid == 321 || pid == 2212);
    if(!(isNeg || isPos))
      return true;

    float const torus       = configBank.getFloat("torus", 0);
    bool const electron_out = (torus == 1.0f);
    bool const particle_inb = (electron_out ? isPos : isNeg);
    bool const particle_out = !particle_inb;

    double const px    = particleBank.getFloat("px", pindex);
    double const py    = particleBank.getFloat("py", pindex);
    double const pz    = particleBank.getFloat("pz", pindex);
    double const rho   = std::hypot(px, py);
    double const theta = std::atan2(rho, (pz == 0.0 ? 1e-12 : pz)) * (180.0 / M_PI);

    double e1 = 0.0, e2 = 0.0, e3 = 0.0;
    bool saw_dc = false;

    auto const& traj = *trajBank;
    int const n      = traj.getRows();
    for(int i = 0; i < n; ++i) {
      if(traj.getInt("pindex", i) != pindex)
        continue;
      if(traj.getInt("detector", i) != DetectorType::DC)
        continue;
      saw_dc          = true;
      int const layer = traj.getInt("layer", i);
      double const e  = traj.getFloat("edge", i);
      if(layer == 6)
        e1 = e;
      else if(layer == 18)
        e2 = e;
      else if(layer == 36)
        e3 = e;
    }

    if(!saw_dc)
      return true;

    auto pass3 = [](double a1, double a2, double a3, double t1, double t2, double t3) -> bool {
      return (a1 > t1 && a2 > t2 && a3 > t3);
    };

    if(particle_inb) {
      if(theta < m_dc.theta_small_deg) {
        return pass3(e1, e2, e3, m_dc.in_small_e1, m_dc.in_small_e2, m_dc.in_small_e3);
      }
      return pass3(e1, e2, e3, m_dc.in_large_e1, m_dc.in_large_e2, m_dc.in_large_e3);
    }
    else if(particle_out) {
      return pass3(e1, e2, e3, m_dc.out_e1, m_dc.out_e2, m_dc.out_e3);
    }
    return true;
  }

  bool FiducialFilterPass2::Filter(int track_index, hipo::bank const& particleBank,
                                   hipo::bank const& configBank, hipo::bank const* calBank,
                                   hipo::bank const* trajBank, hipo::bank const* ftBank) const
  {

    int const pid        = particleBank.getInt("pid", track_index);
    int const strictness = m_cal_strictness;

    auto has_assoc = [&track_index](hipo::bank const* b) -> bool {
      if(!b)
        return false;
      int const n = b->getRows();
      for(int i = 0; i < n; ++i)
        if(b->getInt("pindex", i) == track_index)
          return true;
      return false;
    };

    bool const hasCal = has_assoc(calBank);
    bool const hasFT  = has_assoc(ftBank);
    bool const hasCVT = traj_has_detector(trajBank, track_index, DetectorType::CVT);
    bool const hasDC  = traj_has_detector(trajBank, track_index, DetectorType::DC);

    bool pass = true;

    if(pid == 11 || pid == -11) {
      if(hasFT) {
        pass = pass && PassFTFiducial(track_index, ftBank);
      }
      else {
        if(hasCal) {
          auto calhits = CollectCalHitsForTrack(*calBank, track_index);
          pass         = pass && PassCalStrictness(calhits, strictness);
        }
        if(hasDC) {
          pass = pass && PassDCFiducial(track_index, particleBank, configBank, trajBank);
        }
      }
      return pass;
    }

    if(pid == 22) {
      if(hasFT) {
        pass = pass && PassFTFiducial(track_index, ftBank);
      }
      else if(hasCal) {
        auto calhits = CollectCalHitsForTrack(*calBank, track_index);
        pass         = pass && PassCalStrictness(calhits, strictness);
      }
      return pass;
    }

    if(pid == 211 || pid == 321 || pid == 2212 ||
       pid == -211 || pid == -321 || pid == -2212) {
      if(hasCVT)
        pass = pass && PassCVTFiducial(track_index, trajBank);
      if(hasDC)
        pass = pass && PassDCFiducial(track_index, particleBank, configBank, trajBank);
      return pass;
    }

    return true;
  }

}
