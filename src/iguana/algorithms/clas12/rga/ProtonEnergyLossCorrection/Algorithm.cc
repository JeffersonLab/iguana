#include "Algorithm.h"

#include <cmath>
#include <stdexcept>

#include <yaml-cpp/yaml.h>

namespace iguana::clas12::rga {

  REGISTER_IGUANA_ALGORITHM(ProtonEnergyLossCorrection);

  namespace {
    // Avoid relying on non-portable M_PI.
    static constexpr double kPi = 3.14159265358979323846;
    static constexpr double kRadToDeg = 180.0 / kPi;
    static constexpr double kDegToRad = kPi / 180.0;

    // Helper: keep an angle in degrees within [0,360).
    double WrapDeg360(double x)
    {
      while(x >= 360.0) {
        x -= 360.0;
      }
      while(x < 0.0) {
        x += 360.0;
      }
      return x;
    }
  } // namespace

  // -----------------------------------------------------------------------------
  // Detector-region helpers (based on REC::Particle status)
  // -----------------------------------------------------------------------------
  //
  // Iguana / CLAS12 conventions:
  // - Forward Detector tracks typically have status in [2000, 4000).
  // - Central Detector tracks typically have status in [4000, 5000).
  //
  // We use abs(status) because negative values can appear depending on
  // reconstruction conventions.
  bool ProtonEnergyLossCorrection::IsFD(int status)
  {
    int s = std::abs(status);
    return (s >= 2000 && s < 4000);
  }

  bool ProtonEnergyLossCorrection::IsCD(int status)
  {
    int s = std::abs(status);
    return (s >= 4000 && s < 5000);
  }

  // -----------------------------------------------------------------------------
  // Small math helpers (p, theta, phi)
  // -----------------------------------------------------------------------------
  double ProtonEnergyLossCorrection::Pmag(double px, double py, double pz)
  {
    return std::sqrt(px * px + py * py + pz * pz);
  }

  // theta from momentum vector, in degrees.
  // We compute cos(theta) = pz / |p| and clamp to [-1,1] for numerical safety.
  double ProtonEnergyLossCorrection::ThetaDeg(double px, double py, double pz)
  {
    double r = Pmag(px, py, pz);
    if(r <= 0.0) {
      return 0.0;
    }
    double c = pz / r;
    if(c > 1.0) {
      c = 1.0;
    }
    if(c < -1.0) {
      c = -1.0;
    }
    return kRadToDeg * std::acos(c);
  }

  // phi convention matching the Java implementation:
  //
  //   phi = toDegrees(atan2(px, py));
  //   phi = phi - 90;
  //   if (phi < 0) phi = 360 + phi;
  //   phi = 360 - phi;
  //
  // This yields phi in [0,360).
  double ProtonEnergyLossCorrection::PhiDeg(double px, double py)
  {
    double phi = kRadToDeg * std::atan2(px, py);
    phi        = phi - 90.0;
    if(phi < 0.0) {
      phi = 360.0 + phi;
    }
    phi = 360.0 - phi;
    return WrapDeg360(phi);
  }

  // Convert from spherical-like (p, theta_deg, phi_deg) back to Cartesian.
  void ProtonEnergyLossCorrection::SphericalToCartesian(double p,
                                                        double theta_deg,
                                                        double phi_deg,
                                                        double& px,
                                                        double& py,
                                                        double& pz)
  {
    double theta = theta_deg * kDegToRad;
    double phi   = phi_deg * kDegToRad;

    px = p * std::sin(theta) * std::cos(phi);
    py = p * std::sin(theta) * std::sin(phi);
    pz = p * std::cos(theta);
  }

  // Evaluate polynomial c0 + c1*x + c2*x^2 + ...
  double ProtonEnergyLossCorrection::EvalPoly(Poly const& p, double x)
  {
    double sum = 0.0;
    double xn  = 1.0;
    for(double c : p.c) {
      sum += c * xn;
      xn *= x;
    }
    return sum;
  }

  // -----------------------------------------------------------------------------
  // Period lookup
  // -----------------------------------------------------------------------------
  //
  // Given a run number, find the first PeriodDef whose run_ranges contain it.
  // If no match is found, return nullptr and the algorithm leaves the row unchanged.
  ProtonEnergyLossCorrection::PeriodDef const* ProtonEnergyLossCorrection::FindPeriod(int run) const
  {
    for(auto const& kv : m_periods) {
      auto const& def = kv.second;
      for(auto const& rr : def.run_ranges) {
        if(run >= rr.first && run <= rr.second) {
          return &def;
        }
      }
    }
    return nullptr;
  }

  // -----------------------------------------------------------------------------
  // ConfigHook: read Config.yaml and populate m_periods
  // -----------------------------------------------------------------------------
  //
  // YAML schema expected:
  //
  //   clas12:
  //     ProtonEnergyLossCorrection:
  //       periods:
  //         <period_key>:
  //           run_ranges:
  //             - [min, max]
  //             - [min, max]
  //           FD:
  //             A_p: [ ... ]
  //             B_p: [ ... ]
  //             ...
  //           CD:
  //             A_p: [ ... ]
  //             ...
  //
  // For each period, we load the FD and CD RegionCoeffs, each of which is
  // a set of polynomials in theta for A/B/C of p/theta/phi.
  void ProtonEnergyLossCorrection::ConfigHook()
  {
    m_periods.clear();

    // Locate the algorithm's installed Config.yaml, unless overridden by search paths.
    std::string const cfg_path =
      GetConfig()->FindFile("algorithms/clas12/rga/ProtonEnergyLossCorrection/Config.yaml");

    YAML::Node root = YAML::LoadFile(cfg_path);

    // Defensive parsing: provide clear error messages if YAML structure changes.
    if(!root["clas12"]) {
      throw std::runtime_error("ProtonEnergyLossCorrection: YAML missing top-level key 'clas12'");
    }
    if(!root["clas12"]["ProtonEnergyLossCorrection"]) {
      throw std::runtime_error("ProtonEnergyLossCorrection: YAML missing key 'clas12:ProtonEnergyLossCorrection'");
    }

    YAML::Node node = root["clas12"]["ProtonEnergyLossCorrection"];
    if(!node["periods"]) {
      throw std::runtime_error("ProtonEnergyLossCorrection: YAML missing key 'periods' under clas12:ProtonEnergyLossCorrection");
    }

    YAML::Node periods_node = node["periods"];

    // Helper: load a polynomial array by name (e.g. "A_p") from a region node.
    auto load_poly = [](YAML::Node const& region_node, char const* name) -> Poly {
      if(!region_node[name]) {
        throw std::runtime_error(std::string("ProtonEnergyLossCorrection: missing coefficient list '") + name + "'");
      }
      YAML::Node a = region_node[name];
      if(!a.IsSequence()) {
        throw std::runtime_error(std::string("ProtonEnergyLossCorrection: coefficient '") + name + "' is not a YAML sequence");
      }

      Poly p;
      p.c.reserve((size_t)a.size());
      for(auto v : a) {
        p.c.push_back(v.as<double>());
      }
      return p;
    };

    // Helper: load all A/B/C for p/theta/phi for a given detector region node.
    auto load_region = [&](YAML::Node const& rnode) -> RegionCoeffs {
      RegionCoeffs rc;

      rc.A_p = load_poly(rnode, "A_p");
      rc.B_p = load_poly(rnode, "B_p");
      rc.C_p = load_poly(rnode, "C_p");

      rc.A_theta = load_poly(rnode, "A_theta");
      rc.B_theta = load_poly(rnode, "B_theta");
      rc.C_theta = load_poly(rnode, "C_theta");

      rc.A_phi = load_poly(rnode, "A_phi");
      rc.B_phi = load_poly(rnode, "B_phi");
      rc.C_phi = load_poly(rnode, "C_phi");

      return rc;
    };

    // Iterate periods dictionary: key -> data
    for(auto it = periods_node.begin(); it != periods_node.end(); ++it) {
      std::string key = it->first.as<std::string>();
      YAML::Node pnode = it->second;

      PeriodDef def;

      // run_ranges required for mapping run -> period
      if(!pnode["run_ranges"]) {
        throw std::runtime_error("ProtonEnergyLossCorrection: period '" + key + "' missing key 'run_ranges'");
      }

      for(auto rr : pnode["run_ranges"]) {
        if(!rr.IsSequence() || rr.size() != 2) {
          throw std::runtime_error("ProtonEnergyLossCorrection: period '" + key + "': each run_ranges entry must be [min,max]");
        }
        def.run_ranges.push_back({rr[0].as<int>(), rr[1].as<int>()});
      }

      // Both regions must exist.
      if(!pnode["FD"]) {
        throw std::runtime_error("ProtonEnergyLossCorrection: period '" + key + "' missing 'FD' block");
      }
      if(!pnode["CD"]) {
        throw std::runtime_error("ProtonEnergyLossCorrection: period '" + key + "' missing 'CD' block");
      }

      def.fd = load_region(pnode["FD"]);
      def.cd = load_region(pnode["CD"]);

      m_periods[key] = def;
    }
  }

  // -----------------------------------------------------------------------------
  // StartHook: cache bank indices
  // -----------------------------------------------------------------------------
  void ProtonEnergyLossCorrection::StartHook(hipo::banklist& banks)
  {
    m_b_rec_particle = GetBankIndex(banks, "REC::Particle");
    m_b_run_config   = GetBankIndex(banks, "RUN::config");
  }

  // -----------------------------------------------------------------------------
  // RunHook: apply correction to each matching REC::Particle row
  // -----------------------------------------------------------------------------
  bool ProtonEnergyLossCorrection::RunHook(hipo::banklist& banks) const
  {
    auto& rec = GetBank(banks, m_b_rec_particle, "REC::Particle");
    auto& run = GetBank(banks, m_b_run_config, "RUN::config");

    int runnum = run.getInt("run", 0);

    // Loop all rows and correct in place.
    for(auto const& row : rec.getRowList()) {

      // Select protons only.
      int pid = rec.getInt("pid", row);
      if(pid != 2212) {
        continue;
      }

      // Select FD or CD only (based on status).
      int status = rec.getInt("status", row);
      if(!IsFD(status) && !IsCD(status)) {
        continue;
      }

      // Read momentum components.
      double px = rec.getFloat("px", row);
      double py = rec.getFloat("py", row);
      double pz = rec.getFloat("pz", row);

      // Transform returns corrected values (or unchanged if not applicable).
      auto [px_new, py_new, pz_new] = Transform(pid, status, runnum, px, py, pz);

      // Write back to bank.
      rec.putFloat("px", row, static_cast<float>(px_new));
      rec.putFloat("py", row, static_cast<float>(py_new));
      rec.putFloat("pz", row, static_cast<float>(pz_new));
    }

    return true;
  }

  void ProtonEnergyLossCorrection::StopHook()
  {
    // No summary output here by default. Validation and monitoring are done
    // by the separate Validator class.
  }

  // -----------------------------------------------------------------------------
  // Transform: core correction logic (FD vs CD functional forms)
  // -----------------------------------------------------------------------------
  std::tuple<vector_element_t, vector_element_t, vector_element_t>
  ProtonEnergyLossCorrection::Transform(int const pid,
                                        int const status,
                                        int const run,
                                        vector_element_t const px_in,
                                        vector_element_t const py_in,
                                        vector_element_t const pz_in) const
  {
    // Defensive: only protons are corrected.
    if(pid != 2212) {
      return {px_in, py_in, pz_in};
    }

    // Detector region classification from status.
    bool is_fd = IsFD(status);
    bool is_cd = IsCD(status);
    if(!is_fd && !is_cd) {
      return {px_in, py_in, pz_in};
    }

    // Period lookup from run number. If run is not recognized, do nothing.
    PeriodDef const* period = FindPeriod(run);
    if(!period) {
      return {px_in, py_in, pz_in};
    }

    // Compute kinematics from input momentum.
    double px = (double)px_in;
    double py = (double)py_in;
    double pz = (double)pz_in;

    double p = Pmag(px, py, pz);
    if(p <= 0.0) {
      return {px_in, py_in, pz_in};
    }

    double theta = ThetaDeg(px, py, pz);      // degrees
    double phi   = PhiDeg(px, py);    // degrees in [0,360)

    // Choose FD vs CD coefficients.
    RegionCoeffs const& coeffs = is_fd ? period->fd : period->cd;

    // Evaluate A/B/C polynomials at this theta.
    double A_p = EvalPoly(coeffs.A_p, theta);
    double B_p = EvalPoly(coeffs.B_p, theta);
    double C_p = EvalPoly(coeffs.C_p, theta);

    double A_theta = EvalPoly(coeffs.A_theta, theta);
    double B_theta = EvalPoly(coeffs.B_theta, theta);
    double C_theta = EvalPoly(coeffs.C_theta, theta);

    double A_phi = EvalPoly(coeffs.A_phi, theta);
    double B_phi = EvalPoly(coeffs.B_phi, theta);
    double C_phi = EvalPoly(coeffs.C_phi, theta);

    // Apply corrections.
    //
    // Forward Detector (FD):
    //   p_new = p + A_p + B_p/p + C_p/p^2
    //   theta_new = theta + A_theta + B_theta/theta + C_theta/theta^2
    //   phi_new   = phi   + A_phi   + B_phi/phi     + C_phi/phi^2
    //
    // Central Detector (CD):
    //   p_new = p + A_p + B_p*p + C_p*p^2
    //   theta_new and phi_new use the same inverse-angle form as FD.
    double p_new     = p;
    double theta_new = theta;
    double phi_new   = phi;

    if(is_fd) {
      p_new += A_p + (B_p / p) + (C_p / (p * p));

      // Avoid divide-by-zero (theta and phi are in degrees).
      if(theta != 0.0) {
        theta_new += A_theta + (B_theta / theta) + (C_theta / (theta * theta));
      }
      if(phi != 0.0) {
        phi_new += A_phi + (B_phi / phi) + (C_phi / (phi * phi));
      }
    } else {
      // is_cd
      p_new += A_p + (B_p * p) + (C_p * p * p);

      if(theta != 0.0) {
        theta_new += A_theta + (B_theta / theta) + (C_theta / (theta * theta));
      }
      if(phi != 0.0) {
        phi_new += A_phi + (B_phi / phi) + (C_phi / (phi * phi));
      }
    }

    // Keep phi consistent with the Java convention.
    phi_new = WrapDeg360(phi_new);

    // Convert back to Cartesian using the corrected spherical variables.
    double px_out = 0.0;
    double py_out = 0.0;
    double pz_out = 0.0;
    SphericalToCartesian(p_new, theta_new, phi_new, px_out, py_out, pz_out);

    return {px_out, py_out, pz_out};
  }

} // namespace iguana::clas12::rga