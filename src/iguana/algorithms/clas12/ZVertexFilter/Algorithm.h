#pragma once

#include <atomic>
#include "iguana/algorithms/Algorithm.h"

namespace iguana::clas12 {

  /// @brief_algo Filter the `REC::Particle` (or similar) bank by cutting on Z Vertex
  ///
  /// @begin_doc_algo{clas12::ZVertexFilter | Filter}
  /// @input_banks{REC::Particle}
  /// @output_banks{REC::Particle}
  /// @end_doc
  ///
  /// @begin_doc_config
  /// @config_param{cuts | list[double] | lower and upper @f$z@f$-vertex cuts; run-range dependent}
  /// @end_doc
  class ZVertexFilter : public Algorithm
  {

      DEFINE_IGUANA_ALGORITHM(ZVertexFilter, clas12::ZVertexFilter)

    public:

      void Start(hipo::banklist& banks) override;
      void Run(hipo::banklist& banks) const override;
      void Stop() override;

      /// @reload_function
      /// @param runnum run number
      void Reload(int const& runnum) const;

      /// @action_function{scalar filter} checks if the Z Vertex is within specified bounds
      /// @param zvertex the particle Z Vertex to check
      /// @returns `true` if `zvertex` is within specified bounds
      bool Filter(double const zvertex) const;

      /// @returns the current run number
      int GetRunNum() const;
      /// @returns the current z-vertex lower cut
      double GetZcutLower() const;
      /// @returns the current z-vertex upper cut
      double GetZcutUpper() const;

    private:

      /// `hipo::banklist` indices
      hipo::banklist::size_type b_particle, b_config;

      /// Run number
      mutable std::atomic<int> oa_runnum;

      /// Z-vertex cuts
      mutable std::atomic<double> oa_zcut_low, oa_zcut_high;
  };

}
