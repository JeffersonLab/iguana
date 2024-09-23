#pragma once

#include "iguana/algorithms/Algorithm.h"

namespace iguana::clas12 {

  /// @brief_algo Momentum Corrections
  ///
  /// Adapted from <https://clasweb.jlab.org/wiki/index.php/CLAS12_Momentum_Corrections#tab=Correction_Code>
  ///
  /// @begin_doc_algo{clas12::MomentumCorrection | Transformer}
  /// @input_banks{RUN::config, REC::Particle, REC::Particle::Sector}
  /// @output_banks{REC::Particle}
  /// @end_doc
  class MomentumCorrection : public Algorithm
  {

      DEFINE_IGUANA_ALGORITHM(MomentumCorrection, clas12::MomentumCorrection)

    public:

      void Start(hipo::banklist& banks) override;
      void Run(hipo::banklist& banks, concurrent_key_t const thread_id = 0) const override;
      void Stop() override;

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
