#pragma once

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

      /// @action_function{scalar filter} checks if the Z Vertex is within specified bounds if pid is one for which the filter should be applied to
      /// @param zvertex the particle Z Vertex to check
      /// @param pid the particle pid
      /// @returns `true` if `zvertex` is within specified bounds
      bool Filter(double const zvertex, int pid) const;

      /// @returns the current run number
      int GetRunNum() const;
      /// @returns the current z-vertex lower cut
      double GetZcutLower() const;
      /// @returns the current z-vertex upper cut
      double GetZcutUpper() const;

    private:
      /// `hipo::banklist` index for the particle bank
      hipo::banklist::size_type b_particle;

      /// Run number
      int o_runnum;

      /// Z-vertex cut
      std::vector<double> o_zcuts;

      /// pids to apply ZVertexFilter to
      std::vector<int> o_pids;
  };

}
