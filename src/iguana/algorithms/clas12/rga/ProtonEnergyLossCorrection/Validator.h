#pragma once

// ProtonEnergyLossCorrectionValidator
//
// For each processed event:
//   1) Find all protons (pid==2212) in REC::Particle.
//   2) Compute their theta before correction, place them into theta bins.
//   3) Run the ProtonEnergyLossCorrection algorithm in place.
//   4) Recompute momentum magnitude p after correction.
//   5) Accumulate mean p_before and mean p_after per theta bin.
//
// Output
// ------
// In StopHook(), print a table:
//   theta bin range, N, <p_before>, <p_after>, <delta>
//

#include "iguana/algorithms/Validator.h"
#include "iguana/algorithms/AlgorithmSequence.h"

#include <array>
#include <memory>
#include <mutex>

namespace iguana::clas12::rga {

  class ProtonEnergyLossCorrectionValidator : public Validator {
    DEFINE_IGUANA_VALIDATOR(ProtonEnergyLossCorrectionValidator, clas12::rga::ProtonEnergyLossCorrectionValidator)

  private:
    void StartHook(hipo::banklist& banks) override;
    bool RunHook(hipo::banklist& banks) const override;
    void StopHook() override;

  private:
    // Cached bank indices.
    hipo::banklist::size_type m_b_particle{};
    hipo::banklist::size_type m_b_config{};

    std::unique_ptr<AlgorithmSequence> m_algo_seq;

    // Theta binning configuration (degrees).
    // Bins: [5,10), [10,15), ...
    static constexpr double kThetaMinDeg  = 5.0;
    static constexpr double kThetaMaxDeg  = 70.0;
    static constexpr double kThetaStepDeg = 5.0;
    static constexpr int kNBins = (int)((kThetaMaxDeg - kThetaMinDeg) / kThetaStepDeg); 

    // Simple per-bin accumulators.
    struct BinAccum {
      long long n = 0;
      double sum_p_before = 0.0;
      double sum_p_after  = 0.0;
    };

    // accumulate over events -> these must be mutable.
    mutable std::array<BinAccum, kNBins> m_bins{};
    mutable long long m_total_protons_in_range = 0;
    mutable long long m_total_protons_all      = 0;

    // Protect shared accumulation in multi-threaded contexts.
    mutable std::mutex m_mutex{};

  private:
    // Map theta (deg) to bin index.
    static int ThetaBinIndex(double theta_deg);

    // Compute theta (deg) from (px,py,pz).
    static double ThetaDegFromPxPyPz(double px, double py, double pz);
  };

} 