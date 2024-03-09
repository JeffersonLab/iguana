#pragma once

#include "iguana/algorithms/Validator.h"

#include <TCanvas.h>
#include <TFile.h>
#include <TH2.h>

namespace iguana::physics {

  /// @brief `iguana::physics::InclusiveKinematics` validator
  class InclusiveKinematicsValidator : public Validator
  {

      DEFINE_IGUANA_VALIDATOR(InclusiveKinematicsValidator, physics::InclusiveKinematicsValidator)

    public:

      void Start(hipo::banklist& banks) override;
      void Run(hipo::banklist& banks) const override;
      void Stop() override;

    private:

      hipo::banklist::size_type b_result;

      mutable TH2D* Q2vsX;

      TString m_output_file_basename;
      TFile* m_output_file;
  };

}
