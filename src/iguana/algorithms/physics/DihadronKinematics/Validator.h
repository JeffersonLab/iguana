#pragma once

#include "iguana/algorithms/Validator.h"

#include <TCanvas.h>
#include <TFile.h>
#include <TH1.h>
#include <TH2.h>

namespace iguana::physics {

  /// @brief `iguana::physics::DihadronKinematics` validator
  class DihadronKinematicsValidator : public Validator
  {

      DEFINE_IGUANA_VALIDATOR(DihadronKinematicsValidator, physics::DihadronKinematicsValidator)

    public:

      void Start(hipo::banklist& banks) override;
      bool Run(hipo::banklist& banks) const override;
      void Stop() override;

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
