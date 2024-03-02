#pragma once

#include "iguana/algorithms/Validator.h"
#include "iguana/algorithms/clas12/MomentumCorrection.h"

#include <TH2.h>

namespace iguana::clas12 {

  /// @brief `iguana::clas12::MomentumCorrection` validator
  class MomentumCorrectionValidator : public Validator
  {

      DEFINE_IGUANA_VALIDATOR(MomentumCorrectionValidator, clas12::MomentumCorrection)

    public:

      void Start(hipo::banklist& banks) override;
      void Run(hipo::banklist& banks) override;
      void Stop() override;

    private:

      std::unique_ptr<MomentumCorrection> m_algo;

      const int nBins     = 100;
      const double momMax = 12.0;

      const std::vector<std::pair<TString,TString>> momStrings = {
        { "px", "p_{x}" },
        { "py", "p_{y}" },
        { "pz", "p_{z}" }
      };

      std::vector<TH2D*> after_vs_before;

  };

}
