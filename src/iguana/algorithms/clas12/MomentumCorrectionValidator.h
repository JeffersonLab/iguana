#pragma once

#include "iguana/algorithms/TypeDefs.h"
#include "iguana/algorithms/Validator.h"
#include "iguana/algorithms/clas12/SectorFinder.h"

#include <TCanvas.h>
#include <TFile.h>
#include <TH2.h>

namespace iguana::clas12 {

  /// @brief `iguana::clas12::MomentumCorrection` validator
  class MomentumCorrectionValidator : public Validator
  {

      DEFINE_IGUANA_VALIDATOR(MomentumCorrectionValidator, clas12::MomentumCorrectionValidator)

    public:

      void Start(hipo::banklist& banks) override;
      void Run(hipo::banklist& banks) const override;
      void Stop() override;

    private:

      hipo::banklist::size_type b_particle;

      std::unique_ptr<SectorFinder> m_sector_finder;

      double const m_p_max       = 12.0;
      double const m_deltaP_max  = 1.0;
      double const m_deltaP_zoom = 0.2;

      const std::vector<int> u_pdg_list = {
          particle::PDG::electron,
          particle::PDG::pi_plus,
          particle::PDG::pi_minus,
          particle::PDG::proton};

      TString m_output_file_basename;
      TFile* m_output_file;
      mutable std::unordered_map<int, std::vector<TH2D*>> u_deltaPvsP;
  };

}
