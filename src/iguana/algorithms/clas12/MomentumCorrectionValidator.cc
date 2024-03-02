#include "MomentumCorrectionValidator.h"

namespace iguana::clas12 {

  REGISTER_IGUANA_VALIDATOR(MomentumCorrectionValidator);

  void MomentumCorrectionValidator::Start(hipo::banklist& banks)
  {
    for(const auto& [momName, momTitle] : momStrings) {
      after_vs_before.push_back(new TH2D(
          "after_vs_before_" + momName,
          momTitle + " momentum correction;" + momTitle + "^{before} [GeV];" + momTitle + "^{after} [GeV]",
          nBins, 0, momMax,
          nBins, 0, momMax));
    }
  }


  void MomentumCorrectionValidator::Run(hipo::banklist& banks)
  {
  }


  void MomentumCorrectionValidator::Stop()
  {
  }

}
