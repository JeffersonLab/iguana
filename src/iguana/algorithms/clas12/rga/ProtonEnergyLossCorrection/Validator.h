#pragma once

#include "iguana/algorithms/Validator.h"
#include "iguana/algorithms/AlgorithmSequence.h"

#include <array>
#include <memory>
#include <mutex>

namespace iguana::clas12::rga {

  // Validator for clas12::rga::ProtonEnergyLossCorrection
  class ProtonEnergyLossCorrectionValidator : public Validator
  {
      DEFINE_IGUANA_VALIDATOR(ProtonEnergyLossCorrectionValidator, clas12::rga::ProtonEnergyLossCorrectionValidator)

    private:
      void StartHook(hipo::banklist& banks) override;
      bool RunHook(hipo::banklist& banks) const override;
      void StopHook() override;

    private:
      // Banks
      hipo::banklist::size_type b_particle{};
      hipo::banklist::size_type b_config{};

      // Run algorithm via AlgorithmSequence
      std::unique_ptr<AlgorithmSequence> m_algo_seq;

      // Theta binning: [5,10), [10,15), ..., [65,70] (deg)
      static constexpr double kThetaMinDeg  = 5.0;
      static constexpr double kThetaMaxDeg  = 70.0;
      static constexpr double kThetaStepDeg = 5.0;
      static constexpr int kNBins = (int)((kThetaMaxDeg - kThetaMinDeg) / kThetaStepDeg); // 13

      struct BinAccum {
          long long n = 0;
          double sum_p_before = 0.0;
          double sum_p_after  = 0.0;
      };

      // NOTE: RunHook is const, but validators accumulate state -> mark accumulators mutable.
      mutable std::array<BinAccum, kNBins> m_bins{};
      mutable long long m_total_protons_in_range = 0;
      mutable long long m_total_protons_all      = 0;

      mutable std::mutex m_mutex{};

    private:
      static int ThetaBinIndex(double theta_deg);
      static double ThetaDegFromPxPyPz(double px, double py, double pz);
  };

} // namespace iguana::clas12::rga