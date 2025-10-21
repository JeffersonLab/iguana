#pragma once

#include "iguana/algorithms/Algorithm.h"
#include "iguana/algorithms/TypeDefs.h"

namespace iguana::clas12::rga {

  /// @algo_brief{Forward Tagger energy correction}
  /// @algo_type_transformer
  ///
  /// This has been validated only for RG-A Fall 2018 Outbending data. It _may_ also be
  /// appropriate for Pass-2 data from RG-A, RG-B, and RG-K, but they should
  /// be validated.
  class FTEnergyCorrection : public Algorithm {

    DEFINE_IGUANA_ALGORITHM(FTEnergyCorrection, clas12::rga::FTEnergyCorrection)

    public:

      void Start(hipo::banklist& banks) override;
      bool Run(hipo::banklist& banks) const override;
      void Stop() override;

      /// @run_function
      /// @param [in,out] ftParticleBank `RECFT::Particle`, which will have the correction applied
      /// @run_function_returns_true
      bool Run(hipo::bank& ftParticleBank) const;

      /// @action_function{scalar transformer}
      /// Transformation function that returns 4-vector of electron with corrected energy for the Forward Tagger.
      /// Currently only validated for Fall 2018 outbending data.
      /// @param px @f$p_x@f$
      /// @param py @f$p_y@f$
      /// @param pz @f$p_z@f$
      /// @param E @f$E@f$
      /// @returns an electron 4-vector with the corrected energy for the Forward Tagger.
      /// @see `FTEnergyCorrection::CorrectEnergy`
      Momentum4 Transform(
          vector_element_t const px,
          vector_element_t const py,
          vector_element_t const pz,
          vector_element_t const E) const;

      /// @action_function{scalar transformer}
      /// @param E electron energy
      /// @returns the corrected FT electron energy
      /// @see `FTEnergyCorrection::Transform`
      vector_element_t CorrectEnergy(vector_element_t const E) const;

    private:

      hipo::banklist::size_type b_ft_particle;
      double electron_mass;

  };

}
