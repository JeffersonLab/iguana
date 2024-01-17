#pragma once

#include "iguana/algorithms/AlgorithmFactory.h"
#include <tuple>

namespace iguana::clas12 {

  /// @brief Lorentz transform momenta in `REC::Particle` (or similar banks)
  ///
  /// Available frames are:
  /// - `"mirror"`: reverse the momentum (just a demo)
  class LorentzTransformer : public Algorithm {

    DEFINE_IGUANA_ALGORITHM(LorentzTransformer, clas12::LorentzTransformer)

    public:

      /// Lorentz vector element type, matching that of `REC::Particle` momentum components
      using lorentz_element_t = float;

      /// Generic Lorentz vector container type
      using lorentz_vector_t = std::tuple<lorentz_element_t, lorentz_element_t, lorentz_element_t, lorentz_element_t>;

      void Start(hipo::banklist& banks) override;
      void Run(hipo::banklist& banks) const override;
      void Stop() override;

      /// **Action function**: transform the 4-momentum @f$p=(p_x,p_y,p_z,E)@f$ to the specified frame
      /// @param px @f$p_x@f$
      /// @param py @f$p_y@f$
      /// @param pz @f$p_z@f$
      /// @param E @f$E@f$
      /// @returns the transformed momentum
      lorentz_vector_t Transform(
          lorentz_element_t px,
          lorentz_element_t py,
          lorentz_element_t pz,
          lorentz_element_t E
          ) const;

    private:

      /// `hipo::banklist` index for the particle bank
      hipo::banklist::size_type b_particle;

      /// Configuration options
      std::string o_frame;

      /// Lorentz transformation function
      std::function<lorentz_vector_t(lorentz_vector_t)> m_transform;

  };

}
