#pragma once

#include "iguana/algorithms/Algorithm.h"

namespace iguana::clas12 {

  /// @brief Filter the `REC::Particle` (or similar) bank by cutting on Z Vertex
  class ZVertexFilter : public Algorithm
  {

      DEFINE_IGUANA_ALGORITHM(ZVertexFilter, clas12::ZVertexFilter)

    public:

      void Start(hipo::banklist& banks) override;
      void Run(hipo::banklist& banks) const override;
      void Stop() override;

      /// **Action function**: checks if the Z Vertex is within specified bounds
      /// @param zvertex the particle Z Vertex to check
      /// @returns `true` if `zvertex` is within specified bounds
      bool Filter(const double zvertex) const;

    private:
      /// `hipo::banklist` index for the particle bank
      hipo::banklist::size_type b_particle;

      /// Configuration options
      double o_zvertex_low;
      double o_zvertex_high;
  };

}
