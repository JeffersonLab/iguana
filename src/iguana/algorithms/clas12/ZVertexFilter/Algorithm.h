#pragma once

#include "iguana/algorithms/Algorithm.h"
#include "iguana/services/ConcurrentParam.h"

namespace iguana::clas12 {

  /// @algo_brief{Filter the `REC::Particle` (or similar) bank by cutting on Z Vertex}
  /// @algo_type_filter
  ///
  /// @begin_doc_config{clas12/ZVertexFilter}
  /// @config_param{electron_vz | list[double] | lower and upper electron @f$z@f$-vertex cuts; run-range dependent; cuts are not applied to FT electrons (FD and CD only)}
  /// @end_doc
  class ZVertexFilter : public Algorithm
  {

      DEFINE_IGUANA_ALGORITHM(ZVertexFilter, clas12::ZVertexFilter)

    public:

      void Start(hipo::banklist& banks) override;
      bool Run(hipo::banklist& banks) const override;
      void Stop() override;

      /// @run_function
      /// @param [in,out] particleBank `REC::Particle`, which will be filtered
      /// @param [in] configBank `RUN::config`
      /// @returns `false` if all particles are filtered out
      bool Run(hipo::bank& particleBank, hipo::bank const& configBank) const;

      /// @action_function{reload} prepare the event
      /// @when_to_call{for each event}
      /// @param runnum the run number
      /// @returns the key to be used in `::Filter`
      concurrent_key_t PrepareEvent(int const runnum) const;

      /// @action_function{scalar filter} checks if the Z Vertex is within specified bounds if pid is one for which the filter should be applied to.;
      /// Cuts applied to particles in FD or CD (ie not in FT).
      /// @when_to_call{for each particle}
      /// @param zvertex the particle Z Vertex to check
      /// @param pid the particle pid
      /// @param status particle status used to check particle is not in FT
      /// @param key the return value of `::PrepareEvent`
      /// @returns `true` if `zvertex` is within specified bounds
      bool Filter(double const zvertex, int const pid, int const status, concurrent_key_t const key) const;

      /// @param key the return value of `::PrepareEvent`
      /// @returns the current run number
      int GetRunNum(concurrent_key_t const key) const;

      /// @param key the return value of `::PrepareEvent`
      /// @returns the current z-vertex cuts
      std::vector<double> GetElectronZcuts(concurrent_key_t const key) const;

      /// @brief sets the z-vertex cuts
      /// @warning this method is not thread safe; instead, for thread safety,
      /// use `::PrepareEvent` and a custom configuration file.
      /// @param zcut_lower the lower bound of the cut
      /// @param zcut_upper the upper bound of the cut
      /// @param key the, for `::GetElectronZcuts`
      void SetElectronZcuts(double zcut_lower, double zcut_upper, concurrent_key_t const key);

    private:
      hipo::banklist::size_type b_particle, b_config;

      // Reload function
      void Reload(int const runnum, concurrent_key_t key) const;

      /// Run number
      mutable std::unique_ptr<ConcurrentParam<int>> o_runnum;

      /// Electron Z-vertex cuts
      mutable std::unique_ptr<ConcurrentParam<std::vector<double>>> o_electron_vz_cuts;

  };

}
