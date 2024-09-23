#pragma once

#include "iguana/algorithms/Validator.h"

#include <TCanvas.h>
#include <TFile.h>
#include <TH2.h>
#include <TH1.h>

namespace iguana::clas12 {

  /// @brief `iguana::clas12::SectorFinder` validator
  class SectorFinderValidator : public Validator
  {

      DEFINE_IGUANA_VALIDATOR(SectorFinderValidator, clas12::SectorFinderValidator)

    public:

      void Start(hipo::banklist& banks) override;
      void Run(hipo::banklist& banks, concurrent_key_t const thread_id = 0) const override;
      void Stop() override;

    private:

      hipo::banklist::size_type b_particle;
      hipo::banklist::size_type b_sector;
      hipo::banklist::size_type b_cal;

      std::vector<int> const u_pdg_list = {
          particle::PDG::electron,
          particle::PDG::photon,};

      TString m_output_file_basename;
      TFile* m_output_file;
      mutable std::unordered_map<int, std::vector<TH2D*>> u_YvsX;
      TH1D* u_IsInFD;
  };

}
