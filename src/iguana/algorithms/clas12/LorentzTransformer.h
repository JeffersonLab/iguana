#pragma once

#include "iguana/algorithms/AlgorithmFactory.h"
#include <tuple>

namespace iguana::clas12 {

  /// generic Lorentz vector container type; element type is set to match that of `REC::Particle` momentum components
  using lorentz_vector_t = std::tuple<float, float, float, float>;

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
      /// @param p the Lorentz vector @f$p@f$
      /// @returns the transformed momentum
      lorentz_vector_t Transform(lorentz_vector_t p) const;

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
