#pragma once

#include "iguana/algorithms/Algorithm.h"
#include "iguana/algorithms/TypeDefs.h"
#include "iguana/algorithms/clas12/SectorFinder.h"

namespace iguana::clas12 {

  /// @brief Momentum Corrections
  ///
  /// Adapted from <https://clasweb.jlab.org/wiki/index.php/CLAS12_Momentum_Corrections#tab=Correction_Code>
  class MomentumCorrection : public Algorithm
  {

      DEFINE_IGUANA_ALGORITHM(MomentumCorrection, clas12::MomentumCorrection)

    public:

      void Start(hipo::banklist& banks) override;
      void Run(hipo::banklist& banks) const override;
      void Stop() override;

      /// **Action function**: apply the momentum correction
      /// @param px @f$p_x@f$
      /// @param py @f$p_y@f$
      /// @param pz @f$p_z@f$
      /// @param sec the sector
      /// @param pid the particle PDG
      /// @param torus torus setting
      /// @returns the transformed momentum
      vector3_t Transform(vector_element_t px, vector_element_t py, vector_element_t pz, int sec, int pid, float torus) const;

      /// Calculate the correction factor for inbending data
      /// @param Px @f$p_x@f$
      /// @param Py @f$p_y@f$
      /// @param Pz @f$p_z@f$
      /// @param sec the sector
      /// @param pid the particle PDG
      /// @returns the correction factor
      double CorrectionInbending(const vector_element_t Px, const vector_element_t Py, const vector_element_t Pz, const int sec, const int pid) const;

      /// Calculate the correction factor for outbending data
      /// @param Px @f$p_x@f$
      /// @param Py @f$p_y@f$
      /// @param Pz @f$p_z@f$
      /// @param sec the sector
      /// @param pid the particle PDG
      /// @returns the correction factor
      double CorrectionOutbending(const vector_element_t Px, const vector_element_t Py, const vector_element_t Pz, const int sec, const int pid) const;

      /// Energy loss correction for inbending data
      /// @param Px @f$p_x@f$
      /// @param Py @f$p_y@f$
      /// @param Pz @f$p_z@f$
      /// @param pid the particle PDG
      /// @returns the correction factor
      double EnergyLossInbending(const vector_element_t Px, const vector_element_t Py, const vector_element_t Pz, const int pid) const;

      /// Energy loss correction for outbending data
      /// @param Px @f$p_x@f$
      /// @param Py @f$p_y@f$
      /// @param Pz @f$p_z@f$
      /// @param pid the particle PDG
      /// @returns the correction factor
      double EnergyLossOutbending(const vector_element_t Px, const vector_element_t Py, const vector_element_t Pz, const int pid) const;

    private:

      /// `hipo::banklist` index for the particle bank
      hipo::banklist::size_type b_particle;
      /// `hipo::banklist` index for the config bank
      hipo::banklist::size_type b_config;

      /// sector finder
      std::unique_ptr<SectorFinder> m_sector_finder;
  };

}
