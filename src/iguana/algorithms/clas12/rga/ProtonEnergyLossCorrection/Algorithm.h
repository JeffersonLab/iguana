#pragma once

// ProtonEnergyLossCorrection
//
// Apply momentum and angular corrections to reconstructed protons in CLAS12 RGA
// data; runs event-by-event and modifies REC::Particle in place.
// For each row in REC::Particle:
//   - If pid == 2212 (proton) AND status indicates FD or CD,
//     compute p, theta, phi from (px,py,pz), apply period-dependent corrections,
//     then write back corrected (px,py,pz).
//
// Period dependence
// -----------------
// The coefficients used for the corrections depend on the run number.
// The mapping from run -> (coefficients) is defined in Config.yaml.
// Config.yaml provides one or more "periods" blocks, each with:
//   - run_ranges: list of [min,max] run-number ranges
//   - FD coefficients
//   - CD coefficients
//

#include "iguana/algorithms/Algorithm.h"
#include "iguana/algorithms/TypeDefs.h"

#include <map>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace iguana::clas12::rga {

  class ProtonEnergyLossCorrection : public Algorithm
  {
    DEFINE_IGUANA_ALGORITHM(ProtonEnergyLossCorrection, clas12::rga::ProtonEnergyLossCorrection)

  public:
    // @action_function{scalar transformer}
    //
    // Inputs:
    //   pid    : PDG ID (we correct only protons: 2212)
    //   status : REC::Particle status code (used to identify FD vs CD)
    //   run    : run number (selects the period / coefficients)
    //   px,py,pz : momentum components (GeV)
    //
    // Output:
    //   corrected (px,py,pz). 
    std::tuple<vector_element_t, vector_element_t, vector_element_t> Transform(
        int const pid,
        int const status,
        int const run,
        vector_element_t const px,
        vector_element_t const py,
        vector_element_t const pz) const;

  private:
    // Iguana hook API
    // ---------------
    void ConfigHook() override;
    void StartHook(hipo::banklist& banks) override;
    bool RunHook(hipo::banklist& banks) const override;
    void StopHook() override;

    // Cached bank indices (resolved once in StartHook)
    hipo::banklist::size_type m_b_rec_particle{};
    hipo::banklist::size_type m_b_run_config{};

    // Internal configuration representation
    // -------------------------------------
    // We read YAML coefficients into these structs for fast lookup.

    // Simple polynomial container: c0 + c1*x + c2*x^2 + ...
    struct Poly {
      std::vector<double> c;
    };

    // Coefficients for a single detector region (FD or CD).
    //
    // For each of p, theta, phi we store (A,B,C) polynomials in theta:
    //   A(theta), B(theta), C(theta)
    //
    // Then the correction formula uses those A,B,C values.
    struct RegionCoeffs {
      Poly A_p;
      Poly B_p;
      Poly C_p;

      Poly A_theta;
      Poly B_theta;
      Poly C_theta;

      Poly A_phi;
      Poly B_phi;
      Poly C_phi;
    };

    // Full period definition:
    // - run_ranges define which runs map to this period
    // - fd / cd hold coefficients for Forward Detector and Central Detector
    struct PeriodDef {
      std::vector<std::pair<int, int>> run_ranges;
      RegionCoeffs fd;
      RegionCoeffs cd;
    };

    // Map period key -> period definition (loaded from YAML)
    std::map<std::string, PeriodDef> m_periods{};

    // Helper functions (pure utilities)
    static bool IsFD(int status);
    static bool IsCD(int status);

    static double EvalPoly(Poly const& p, double x);

    static double Pmag(double px, double py, double pz);
    static double ThetaDeg(double px, double py, double pz);

    // phi convention that matches the Java implementation (see header comment).
    static double PhiDeg(double px, double py);

    // Convert (p, theta_deg, phi_deg) back to (px,py,pz).
    static void SphericalToCartesian(double p, double theta_deg, double phi_deg,
      double& px, double& py, double& pz);

    // Given a run number, return the matching run period (e.g. Fa18 Inb) or nullptr.
    PeriodDef const* FindPeriod(int run) const;
  };

}