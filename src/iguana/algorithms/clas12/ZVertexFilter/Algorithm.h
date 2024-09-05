#pragma once

#include "iguana/algorithms/Algorithm.h"
#include "iguana/services/ConcurrentParam.h"

namespace iguana::clas12 {

  /// @brief_algo Filter the `REC::Particle` (or similar) bank by cutting on Z Vertex
  ///
  /// @begin_doc_algo{clas12::ZVertexFilter | Filter}
  /// @input_banks{REC::Particle, RUN::config}
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

      /// @action_function{reload} prepare the event
      /// @when_to_call{for each event}
      /// @param runnum the run number
      /// @param key @key_desc
      /// @returns the key to be used in `::Filter`
      concurrent_key_t PrepareEvent(int const runnum, concurrent_key_t key = 0) const;

      /// @action_function{scalar filter} checks if the Z Vertex is within specified bounds if pid is one for which the filter should be applied to.;
      /// Cuts applied to particles in FD or CD (ie not in FT).
      /// @when_to_call{for each particle}
      /// @param zvertex the particle Z Vertex to check
      /// @param pid the particle pid
      /// @param status particle status used to check particle is not in FT
      /// @param key the return value of `::PrepareEvent`
      /// @returns `true` if `zvertex` is within specified bounds
      bool Filter(double const zvertex, int const pid, int const status, concurrent_key_t const key) const;

      /// @returns the current run number
      int GetRunNum() const;
      /// @returns the current z-vertex lower cut
      double GetZcutLower() const;
      /// @returns the current z-vertex upper cut
      double GetZcutUpper() const;

    private:
      hipo::banklist::size_type b_particle, b_config;

      // Reload function
      void Reload(int const runnum, concurrent_key_t key = 0) const;

      /// Run number
      mutable std::unique_ptr<ConcurrentParam<int>> o_runnum;

      /// Z-vertex cut
      mutable std::unique_ptr<ConcurrentParam<std::vector<double>>> o_zcuts;

      /// pids to apply ZVertexFilter to
      std::set<int> o_pids;
  };

}
