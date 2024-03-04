#pragma once

#include "iguana/algorithms/Validator.h"
#include "iguana/algorithms/TypeDefs.h"

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

      const int m_num_bins   = 100;
      const double m_mom_max = 12.0;

      const std::vector<int> u_pdg_list = {
        particle::PDG::electron,
        particle::PDG::pi_plus,
        particle::PDG::pi_minus,
        particle::PDG::proton};

      const std::vector<std::pair<TString, TString>> u_mom_strings = {
          {"px", "p_{x}"},
          {"py", "p_{y}"},
          {"pz", "p_{z}"}};

      mutable std::unordered_map<int, std::vector<TH2D*>> u_after_vs_before;
  };

}
