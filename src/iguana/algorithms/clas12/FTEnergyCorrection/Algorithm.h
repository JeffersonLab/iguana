#pragma once

#include "iguana/algorithms/Algorithm.h"
#include "iguana/algorithms/TypeDefs.h"

namespace iguana::clas12 {

  /// @brief_algo Forward Tagger energy correction
  ///
  /// @begin_doc_algo{clas12::FTEnergyCorrection | Transformer}
  /// @input_banks{RECFT::Particle}
  /// @output_banks{RECFT::Particle}
  /// @end_doc
  class FTEnergyCorrection : public Algorithm {

    DEFINE_IGUANA_ALGORITHM(FTEnergyCorrection, clas12::FTEnergyCorrection)

    public:

      void Start(hipo::banklist& banks) override;
      void Run(hipo::banklist& banks) const override;
      void Stop() override;

      /// @action_function{scalar transformer}
      /// Transformation function that returns 4-vector of electron with corrected energy for the Forward Tagger.
      /// Currently only validated for Fall 2018 outbending data.
      /// @param px @f$p_x@f$
      /// @param py @f$p_y@f$
      /// @param pz @f$p_z@f$
      /// @param E @f$E@f$
      /// @returns an electron 4-vector with the corrected energy for the Forward Tagger.
      /// @see `FTEnergyCorrection::CorrectEnergy`
      vector4_t Transform(
          vector_element_t px,
          vector_element_t py,
          vector_element_t pz,
          vector_element_t E) const;

      /// @action_function{scalar transformer}
      /// @param E electron energy
      /// @returns the corrected FT electron energy
      /// @see `FTEnergyCorrection::Transform`
      vector_element_t CorrectEnergy(vector_element_t E) const;

    private:

      hipo::banklist::size_type b_ft_particle;
      double electron_mass;

  };

}
