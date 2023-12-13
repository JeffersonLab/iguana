#pragma once

#include "iguana/Algorithm.h"
#include "iguana/AlgorithmFactory.h"

namespace iguana::clas12 {

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

      void Start(hipo::banklist& banks) override;
      void Run(hipo::banklist& banks) const override;
      void Stop() override;

      /// **Action function**: transform the 4-momentum @f$p=(p_x,p_y,p_z,E)@f$ to the specified frame
      /// @param px @f$p_x@f$
      /// @param py @f$p_y@f$
      /// @param pz @f$p_z@f$
      /// @param e @f$E@f$
      void Transform(float& px, float& py, float& pz, float& e) const;

    private:

      /// True if this algorithm is registered in `AlgorithmFactory`
      static bool s_registered;

      /// `hipo::banklist` index for the particle bank
      hipo::banklist::size_type b_particle;

      /// Configuration options
      std::string o_frame;

      /// Lorentz transformation function
      std::function<void(float&,float&,float&,float&)> m_transform;

  };

}
