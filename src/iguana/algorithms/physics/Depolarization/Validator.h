#pragma once

#include "iguana/algorithms/Validator.h"

#include <TCanvas.h>
#include <TFile.h>
#include <TH2.h>

namespace iguana::physics {

  /// @brief `iguana::physics::Depolarization` validator
  class DepolarizationValidator : public Validator
  {

      DEFINE_IGUANA_VALIDATOR(DepolarizationValidator, physics::DepolarizationValidator)

    public:

      void Start(hipo::banklist& banks) override;
      void Run(hipo::banklist& banks) const override;
      void Stop() override;

    private:

      hipo::banklist::size_type b_inc_kin;
      hipo::banklist::size_type b_depol;

      struct Plot2D {
        TH2D* hist;
        std::function<double(hipo::bank const&, int const)> get_val;
      };
      std::vector<Plot2D> plots_vs_Q2;
      std::vector<Plot2D> plots_vs_x;
      std::vector<Plot2D> plots_vs_y;

      TString m_output_file_basename;
      TFile* m_output_file;
  };

}
