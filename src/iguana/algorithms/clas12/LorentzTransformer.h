#pragma once

#include "iguana/algorithms/AlgorithmFactory.h"
#include <tuple>

namespace iguana::clas12 {

  /// Lorentz vector element type, matching that of `REC::Particle` momentum components
  using lorentz_element_t = float;

  /// generic Lorentz vector container type
  using lorentz_vector_t = std::tuple<lorentz_element_t, lorentz_element_t, lorentz_element_t, lorentz_element_t>;

  /// @brief Lorentz transform momenta in `REC::Particle` (or similar banks)
  ///
  /// Available frames are:
  /// - `"mirror"`: reverse the momentum (just a demo)
  class LorentzTransformer : public Algorithm {

    public:

      /// @see `Algorithm::Algorithm`
      LorentzTransformer(std::string name="") : Algorithm(name=="" ? ClassName() : name) {}
      ~LorentzTransformer() {}
      /// @returns An instance of this algorithm. This is used by `AlgorithmFactory`.
      static algo_t Creator() { return std::make_unique<LorentzTransformer>(); }
      /// @returns This algorithm's class name
      static std::string ClassName() { return "clas12::LorentzTransformer"; }

      using Algorithm::Start;
      void Start(hipo::banklist& banks) override;
      void Run(hipo::banklist& banks) const override;
      void Stop() override;

      /// **Action function**: transform the 4-momentum @f$p=(p_x,p_y,p_z,E)@f$ to the specified frame
      /// @param px @f$p_x@f$
      /// @param py @f$p_y@f$
      /// @param pz @f$p_z@f$
      /// @param E @f$E@f$
      /// @returns the transformed momentum
      lorentz_vector_t Transform(lorentz_element_t px, lorentz_element_t py, lorentz_element_t pz, lorentz_element_t E) const;

    private:

      /// True if this algorithm is registered in `AlgorithmFactory`
      static bool s_registered;

      /// `hipo::banklist` index for the particle bank
      hipo::banklist::size_type b_particle;

      /// Configuration options
      std::string o_frame;

      /// Lorentz transformation function
      std::function<lorentz_vector_t(lorentz_vector_t)> m_transform;

  };

}
