#pragma once

#include "iguana/algorithms/Algorithm.h"

namespace iguana::clas12 {

  /// @brief_algo Filter the `REC::Particle` (or similar) bank by PID from the Event Builder
  ///
  /// @begin_doc_algo{clas12::EventBuilderFilter | Filter}
  /// <tr><td>see this algorithm's Run function(s) for the input and output bank names</td></tr>
  /// @end_doc
  ///
  /// @begin_doc_config{clas12/EventBuilderFilter}
  /// @config_param{pids | list[int] | list of PDG codes to filter}
  /// @end_doc
  class EventBuilderFilter : public Algorithm
  {

      DEFINE_IGUANA_ALGORITHM(EventBuilderFilter, clas12::EventBuilderFilter)

    public:

      void Start(hipo::banklist& banks) override;
      void Run(hipo::banklist& banks) const override;
      void Stop() override;

      /// run function
      /// @param [in,out] particleBank `REC::Particle`, which will be filtered
      void Run(hipo::bank& particleBank) const;

      /// @action_function{scalar filter} checks if the PDG `pid` is a part of the list of user-specified PDGs
      /// @param pid the particle PDG to check
      /// @returns `true` if `pid` is one the user wants
      bool Filter(int const pid) const;

      /// @action_function{vector filter} checks if the PDG `pid` is a part of the list of user-specified PDGs
      /// @overloads_scalar
      /// @param pids the list of particle PDGs to check
      /// @returns list of booleans which are `true` for `pids` the user wants
      std::deque<bool> Filter(std::vector<int> const pids) const;

    private:

      /// `hipo::banklist` index for the particle bank
      hipo::banklist::size_type b_particle;

      /// Configuration options
      std::set<int> o_pids;
  };

}
