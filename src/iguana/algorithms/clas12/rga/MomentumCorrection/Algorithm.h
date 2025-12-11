#pragma once

#include "iguana/algorithms/Algorithm.h"
#include "iguana/algorithms/TypeDefs.h"

namespace iguana::clas12::rga {

  /// @algo_brief{Momentum Corrections}
  /// @algo_type_transformer
  /// Adapted from <https://clasweb.jlab.org/wiki/index.php/CLAS12_Momentum_Corrections#tab=Correction_Code>
  class MomentumCorrection : public Algorithm
  {

      DEFINE_IGUANA_ALGORITHM(MomentumCorrection, clas12::rga::MomentumCorrection)

    public:

      void Start(hipo::banklist& banks) override;
      bool Run(hipo::banklist& banks) const override;
      void Stop() override;

      /// @run_function
      /// @param [in,out] particleBank `REC::Particle`; the momenta will be corrected
      /// @param [in] sectorBank `REC::Particle::Sector`, from `SectorFinder`
      /// @param [in] configBank `RUN::config`
      /// @run_function_returns_true
      bool Run(
          hipo::bank& particleBank,
          hipo::bank const& sectorBank,
          hipo::bank const& configBank) const;

      /// @action_function{scalar transformer} Apply the momentum correction
      /// @param px @f$p_x@f$
      /// @param py @f$p_y@f$
      /// @param pz @f$p_z@f$
      /// @param sec the sector
      /// @param pid the particle PDG
      /// @param torus torus setting
      /// @returns the transformed momentum
      Momentum3 Transform(vector_element_t const px, vector_element_t const py, vector_element_t const pz, int const sec, int const pid, float const torus) const;

      /// @action_function{scalar creator} Calculate the correction factor for inbending data
      /// @param px @f$p_x@f$
      /// @param py @f$p_y@f$
      /// @param pz @f$p_z@f$
      /// @param sec the sector
      /// @param pid the particle PDG
      /// @returns the correction factor
      double CorrectionInbending(vector_element_t const px, vector_element_t const py, vector_element_t const pz, int const sec, int const pid) const;

      /// @action_function{scalar creator} Calculate the correction factor for outbending data
      /// @param px @f$p_x@f$
      /// @param py @f$p_y@f$
      /// @param pz @f$p_z@f$
      /// @param sec the sector
      /// @param pid the particle PDG
      /// @returns the correction factor
      double CorrectionOutbending(vector_element_t const px, vector_element_t const py, vector_element_t const pz, int const sec, int const pid) const;

      /// @action_function{scalar creator} Energy loss correction for inbending data
      /// @param px @f$p_x@f$
      /// @param py @f$p_y@f$
      /// @param pz @f$p_z@f$
      /// @param pid the particle PDG
      /// @returns the correction factor
      double EnergyLossInbending(vector_element_t const px, vector_element_t const py, vector_element_t const pz, int const pid) const;

      /// @action_function{scalar creator} Energy loss correction for outbending data
      /// @param px @f$p_x@f$
      /// @param py @f$p_y@f$
      /// @param pz @f$p_z@f$
      /// @param pid the particle PDG
      /// @returns the correction factor
      double EnergyLossOutbending(vector_element_t const px, vector_element_t const py, vector_element_t const pz, int const pid) const;

    private:

      /// `hipo::banklist` indices
      hipo::banklist::size_type b_particle;
      hipo::banklist::size_type b_sector;
      hipo::banklist::size_type b_config;
  };

}
