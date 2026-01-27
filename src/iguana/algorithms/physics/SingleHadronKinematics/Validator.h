#pragma once

#include "iguana/algorithms/Validator.h"

#include <TCanvas.h>
#include <TFile.h>
#include <TH1.h>
#include <TH2.h>

namespace iguana::physics {

  /// @brief `iguana::physics::SingleHadronKinematics` validator
  class SingleHadronKinematicsValidator : public Validator
  {

      DEFINE_IGUANA_VALIDATOR(SingleHadronKinematicsValidator, physics::SingleHadronKinematicsValidator)

    private: // hooks
      void StartHook(hipo::banklist& banks) override;
      bool RunHook(hipo::banklist& banks) const override;
      void StopHook() override;

    private:

      hipo::banklist::size_type b_result;

      struct Plot1D {
          TH1D* hist;
          std::function<double(hipo::bank const&, int const)> get_val;
      };
      std::vector<Plot1D> plot_list;

      TString m_output_file_basename;
      TFile* m_output_file;
  };

}
