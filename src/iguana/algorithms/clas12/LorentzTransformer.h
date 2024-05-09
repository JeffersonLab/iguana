#pragma once

#include "iguana/algorithms/Algorithm.h"
#include "iguana/algorithms/TypeDefs.h"

namespace iguana::clas12 {

  /// @brief_algo Lorentz transform momenta in `REC::Particle` (or similar banks)
  ///
  /// @begin_doc_algo{Transformer}
  /// @input_banks{REC::Particle}
  /// @output_banks{REC::Particle}
  /// @end_doc
  ///
  /// @begin_doc_config
  /// @config_param{frame | string | the frame to transform to; see below for available frames}
  /// @end_doc
  ///
  /// @par Available Frames:
  /// | Frame Name        | Description                     |
  /// | ---               | ---                             |
  /// | `beam_rest_frame` | rest frame of the electron beam |
  class LorentzTransformer : public Algorithm
  {

      DEFINE_IGUANA_ALGORITHM(LorentzTransformer, clas12::LorentzTransformer)

    public:

      void Start(hipo::banklist& banks) override;
      void Run(hipo::banklist& banks) const override;
      void Stop() override;

      /// @action_function{scalar transformer} boost the 4-momentum @f$p=(p_x,p_y,p_z,E)@f$ along @f$\beta=(\beta_x, \beta_y, \beta_z)@f$
      /// @param p_x @f$p_x@f$
      /// @param p_y @f$p_y@f$
      /// @param p_z @f$p_z@f$
      /// @param E @f$E@f$
      /// @param beta_x @f$\beta_x@f$
      /// @param beta_y @f$\beta_y@f$
      /// @param beta_z @f$\beta_z@f$
      /// @returns the transformed momentum
      vector4_t Boost(
          vector_element_t p_x,
          vector_element_t p_y,
          vector_element_t p_z,
          vector_element_t E,
          vector_element_t beta_x,
          vector_element_t beta_y,
          vector_element_t beta_z) const;

    private:

      /// `hipo::banklist` index for the particle bank
      hipo::banklist::size_type b_particle;

      /// Frame choice
      std::string o_frame;
      /// Beam energy
      double o_beam_energy;

      /// Transformation types
      enum e_transformation_type { e_boost };
      /// Transformation type
      e_transformation_type m_transformation_type;
  };

}
