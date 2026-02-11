#include "Algorithm.h"

#include <cmath>
#include <stdexcept>

#include <yaml-cpp/yaml.h>

namespace iguana::clas12 {

  REGISTER_IGUANA_ALGORITHM(ProtonEnergyLossCorrection);

  // --------------------------
  // Small math helpers
  // --------------------------
  bool ProtonEnergyLossCorrection::IsFD(int status) {
    int s = std::abs(status);
    return (s >= 2000 && s < 4000);
  }

  bool ProtonEnergyLossCorrection::IsCD(int status) {
    int s = std::abs(status);
    return (s >= 4000 && s < 5000);
  }

  double ProtonEnergyLossCorrection::Pmag(double px, double py, double pz) {
    return std::sqrt(px * px + py * py + pz * pz);
  }

  double ProtonEnergyLossCorrection::ThetaDeg(double px, double py, double pz) {
    double r = Pmag(px, py, pz);
    if (r <= 0.0) {
      return 0.0;
    }
    double c = pz / r;
    if (c > 1.0) {
      c = 1.0;
    }
    if (c < -1.0) {
      c = -1.0;
    }
    return (180.0 / M_PI) * std::acos(c);
  }

  //   phi = toDegrees(atan2(x, y));
  //   phi = phi - 90;
  //   if (phi < 0) phi = 360 + phi;
  //   phi = 360 - phi;
  // Returns phi in [0,360).
  double ProtonEnergyLossCorrection::PhiDegLikeJava(double px, double py) {
    double phi = (180.0 / M_PI) * std::atan2(px, py);
    phi = phi - 90.0;
    if (phi < 0.0) {
      phi = 360.0 + phi;
    }
    phi = 360.0 - phi;
    // keep in [0,360)
    while (phi >= 360.0) {
      phi -= 360.0;
    }
    while (phi < 0.0) {
      phi += 360.0;
    }
    return phi;
  }

  void ProtonEnergyLossCorrection::SphericalToCartesian(double p, double theta_deg, double phi_deg, double& px, double& py, double& pz) {
    double theta = theta_deg * (M_PI / 180.0);
    double phi = phi_deg * (M_PI / 180.0);
    px = p * std::sin(theta) * std::cos(phi);
    py = p * std::sin(theta) * std::sin(phi);
    pz = p * std::cos(theta);
  }

  double ProtonEnergyLossCorrection::EvalPoly(Poly const& p, double x) {
    double sum = 0.0;
    double xn = 1.0;
    for (double c : p.c) {
      sum += c * xn;
      xn *= x;
    }
    return sum;
  }

  ProtonEnergyLossCorrection::PeriodDef const* ProtonEnergyLossCorrection::FindPeriod(int run) const {
    for (auto const& kv : periods_) {
      auto const& def = kv.second;
      for (auto const& rr : def.run_ranges) {
        if (run >= rr.first && run <= rr.second) {
          return &def;
        }
      }
    }
    return nullptr;
  }

  // --------------------------
  // Hooks
  // --------------------------
  void ProtonEnergyLossCorrection::ConfigHook() {
    periods_.clear();

    // If your branch uses a helper like GetConfigYAML(), swap this accordingly.
    // For now we use the conventional path from Algorithm::GetConfigPath().
    std::string cfg_path = GetConfig()->FindFile("Config.yaml");

    YAML::Node root = YAML::LoadFile(cfg_path);

    if (!root["clas12"] || !root["clas12"]["ProtonEnergyLossCorrection"]) {
      throw std::runtime_error("ProtonEnergyLossCorrection: missing YAML node clas12:ProtonEnergyLossCorrection");
    }

    YAML::Node node = root["clas12"]["ProtonEnergyLossCorrection"];
    if (!node["periods"]) {
      throw std::runtime_error("ProtonEnergyLossCorrection: missing YAML node periods");
    }

    auto periods_node = node["periods"];
    for (auto it = periods_node.begin(); it != periods_node.end(); ++it) {
      std::string key = it->first.as<std::string>();
      YAML::Node pnode = it->second;

      PeriodDef def;

      if (!pnode["run_ranges"]) {
        throw std::runtime_error("ProtonEnergyLossCorrection: missing run_ranges for period " + key);
      }
      for (auto rr : pnode["run_ranges"]) {
        if (!rr.IsSequence() || rr.size() != 2) {
          throw std::runtime_error("ProtonEnergyLossCorrection: run_ranges entry must be [min,max] for period " + key);
        }
        def.run_ranges.push_back({rr[0].as<int>(), rr[1].as<int>()});
      }

      auto load_poly = [](YAML::Node const& n, char const* name) -> Poly {
        Poly p;
        if (!n[name]) {
          throw std::runtime_error(std::string("ProtonEnergyLossCorrection: missing coeff node ") + name);
        }
        YAML::Node a = n[name];
        if (!a.IsSequence()) {
          throw std::runtime_error(std::string("ProtonEnergyLossCorrection: coeff node not sequence: ") + name);
        }
        for (auto v : a) {
          p.c.push_back(v.as<double>());
        }
        return p;
      };

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

      if (!pnode["FD"] || !pnode["CD"]) {
        throw std::runtime_error("ProtonEnergyLossCorrection: missing FD or CD node for period " + key);
      }

      def.fd = load_region(pnode["FD"]);
      def.cd = load_region(pnode["CD"]);

      periods_[key] = def;
    }
  }

  void ProtonEnergyLossCorrection::StartHook(hipo::banklist& banks) {
    b_rec_particle = GetBankIndex(banks, "REC::Particle");
    b_run_config = GetBankIndex(banks, "RUN::config");
  }

  bool ProtonEnergyLossCorrection::RunHook(hipo::banklist& banks) const {
    auto& rec = GetBank(banks, b_rec_particle, "REC::Particle");
    auto& run = GetBank(banks, b_run_config, "RUN::config");

    int runnum = run.getInt("run", 0);

    for (auto const& row : rec.getRowList()) {
      int pid = rec.getInt("pid", row);
      if (pid != 2212) {
        continue;
      }

      int status = rec.getInt("status", row);
      if (!IsFD(status) && !IsCD(status)) {
        continue;
      }

      double px = rec.getFloat("px", row);
      double py = rec.getFloat("py", row);
      double pz = rec.getFloat("pz", row);

      auto [px_new, py_new, pz_new] = Transform(pid, status, runnum, px, py, pz);

      rec.putFloat("px", row, static_cast<float>(px_new));
      rec.putFloat("py", row, static_cast<float>(py_new));
      rec.putFloat("pz", row, static_cast<float>(pz_new));
    }

    return true;
  }

  void ProtonEnergyLossCorrection::StopHook() {
  }

  // --------------------------
  // Transform
  // --------------------------
  std::tuple<vector_element_t, vector_element_t, vector_element_t> ProtonEnergyLossCorrection::Transform(
      int const pid,
      int const status,
      int const run,
      vector_element_t const px_in,
      vector_element_t const py_in,
      vector_element_t const pz_in) const
  {
    // Only protons
    if (pid != 2212) {
      return {px_in, py_in, pz_in};
    }

    bool is_fd = IsFD(status);
    bool is_cd = IsCD(status);
    if (!is_fd && !is_cd) {
      return {px_in, py_in, pz_in};
    }

    PeriodDef const* period = FindPeriod(run);
    if (!period) {
      return {px_in, py_in, pz_in};
    }

    double px = px_in;
    double py = py_in;
    double pz = pz_in;

    double p = Pmag(px, py, pz);
    if (p <= 0.0) {
      return {px_in, py_in, pz_in};
    }

    double theta = ThetaDeg(px, py, pz);
    double phi = PhiDegLikeJava(px, py);

    RegionCoeffs const& c = is_fd ? period->fd : period->cd;

    // Build A/B/C from theta polynomials
    double A_p = EvalPoly(c.A_p, theta);
    double B_p = EvalPoly(c.B_p, theta);
    double C_p = EvalPoly(c.C_p, theta);

    double A_theta = EvalPoly(c.A_theta, theta);
    double B_theta = EvalPoly(c.B_theta, theta);
    double C_theta = EvalPoly(c.C_theta, theta);

    double A_phi = EvalPoly(c.A_phi, theta);
    double B_phi = EvalPoly(c.B_phi, theta);
    double C_phi = EvalPoly(c.C_phi, theta);

    // Apply corrections using your exact functional forms
    double p_new = p;
    double theta_new = theta;
    double phi_new = phi;

    if (is_fd) {
      p_new += A_p + B_p / p + C_p / (p * p);

      // Guard theta,phi divisions
      if (theta != 0.0) {
        theta_new += A_theta + B_theta / theta + C_theta / (theta * theta);
      }
      if (phi != 0.0) {
        phi_new += A_phi + B_phi / phi + C_phi / (phi * phi);
      }
    } else if (is_cd) {
      p_new += A_p + B_p * p + C_p * p * p;

      if (theta != 0.0) {
        theta_new += A_theta + B_theta / theta + C_theta / (theta * theta);
      }
      if (phi != 0.0) {
        phi_new += A_phi + B_phi / phi + C_phi / (phi * phi);
      }
    }

    // phi convention: keep in [0,360) since that is what your Java uses
    while (phi_new >= 360.0) {
      phi_new -= 360.0;
    }
    while (phi_new < 0.0) {
      phi_new += 360.0;
    }

    // Convert back to Cartesian using spherical conversion
    double px_out = 0.0, py_out = 0.0, pz_out = 0.0;
    SphericalToCartesian(p_new, theta_new, phi_new, px_out, py_out, pz_out);

    return {px_out, py_out, pz_out};
  }

} // namespace iguana::clas12