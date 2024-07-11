#pragma once

#include "iguana/algorithms/TypeDefs.h"
#include "iguana/algorithms/Validator.h"

#include <TFile.h>
#include <TH1.h>

namespace iguana::clas12 {

  /// @brief `iguana::clas12::ZVertexFilter` validator
  class ZVertexFilterValidator : public Validator
  {

      DEFINE_IGUANA_VALIDATOR(ZVertexFilterValidator, clas12::ZVertexFilterValidator)

    public:

      void Start(hipo::banklist& banks) override;
      void Run(hipo::banklist& banks) const override;
      void Stop() override;

    private:

      hipo::banklist::size_type b_particle;

      // add pdgs not to cut to check we're 
      // only cutting on right particles
      std::vector<int> const u_pdg_list = {
          particle::PDG::electron,
          particle::PDG::pi_plus,
          particle::PDG::pi_minus,
          particle::PDG::proton,
          particle::PDG::neutron};

      std::vector<int> const u_pdgtocut_list = {
          particle::PDG::electron};

      std::vector<double> const u_cuts_list = {
          -5,
          5};

      TString m_output_file_basename;
      TFile* m_output_file;
      mutable std::unordered_map<int, std::vector<TH1D*>> u_zvertexplots;
  };

}
