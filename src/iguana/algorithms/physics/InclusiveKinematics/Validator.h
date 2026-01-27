#pragma once

#include "iguana/algorithms/Validator.h"

#include <TCanvas.h>
#include <TFile.h>
#include <TH1.h>
#include <TH2.h>

namespace iguana::physics {

  /// @brief `iguana::physics::InclusiveKinematics` validator
  class InclusiveKinematicsValidator : public Validator
  {

    DEFINE_IGUANA_VALIDATOR(InclusiveKinematicsValidator, physics::InclusiveKinematicsValidator)

    private: // hooks
      void StartHook(hipo::banklist& banks) override;
      bool RunHook(hipo::banklist& banks) const override;
      void StopHook() override;

    private:

      hipo::banklist::size_type b_particle;
      hipo::banklist::size_type b_result;

      mutable TH1D* lepton_p_dist;
      mutable TH1D* lepton_theta_dist;
      mutable TH1D* lepton_phi_dist;
      mutable TH1D* lepton_vz_dist;
      mutable TH2D* Q2_vs_x;
      mutable TH2D* Q2_vs_W;
      mutable TH1D* y_dist;
      mutable TH1D* nu_dist;

      TString m_output_file_basename;
      TFile* m_output_file;
  };

}
