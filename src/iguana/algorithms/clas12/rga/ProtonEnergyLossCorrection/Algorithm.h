#pragma once

#include "iguana/algorithms/Algorithm.h"
#include "iguana/algorithms/TypeDefs.h"

#include <map>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace iguana::clas12::rga {

  class ProtonEnergyLossCorrection : public Algorithm {
    DEFINE_IGUANA_ALGORITHM(ProtonEnergyLossCorrection, clas12::rga::ProtonEnergyLossCorrection)

  public:
    // @action_function{scalar transformer}
    // Transform a single particle row (px,py,pz) if it is an RGA proton.
    // Returns corrected (px,py,pz).
    std::tuple<vector_element_t, vector_element_t, vector_element_t> Transform(
        int const pid,
        int const status,
        int const run,
        vector_element_t const px,
        vector_element_t const py,
        vector_element_t const pz) const;

  private:
    // Hook API (Start/Run/Stop are final in base class)
    void ConfigHook() override;
    void StartHook(hipo::banklist& banks) override;
    bool RunHook(hipo::banklist& banks) const override;
    void StopHook() override;

    // Bank indices
    hipo::banklist::size_type b_rec_particle;
    hipo::banklist::size_type b_run_config;

    // Internal config structures
    struct Poly {
      std::vector<double> c; // c0 + c1*x + c2*x^2 + ...
    };

    struct RegionCoeffs {
      Poly A_p, B_p, C_p;
      Poly A_theta, B_theta, C_theta;
      Poly A_phi, B_phi, C_phi;
    };

    struct PeriodDef {
      std::vector<std::pair<int, int>> run_ranges;
      RegionCoeffs fd;
      RegionCoeffs cd;
    };

    // period key -> period def
    std::map<std::string, PeriodDef> periods_;

    // Helpers
    static bool IsFD(int status);
    static bool IsCD(int status);
    static double EvalPoly(Poly const& p, double x);
    static double Pmag(double px, double py, double pz);
    static double ThetaDeg(double px, double py, double pz);
    static double PhiDegLikeJava(double px, double py);
    static void SphericalToCartesian(double p, double theta_deg, double phi_deg, double& px, double& py, double& pz);

    // Find matching period for run, returns nullptr if none
    PeriodDef const* FindPeriod(int run) const;
  };

} // namespace iguana::clas12::rga