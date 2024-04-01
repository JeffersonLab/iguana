#pragma once

#include "iguana/algorithms/Algorithm.h"

namespace iguana::clas12 {

  /// @brief Filter the `REC::Particle` (or similar) bank by PID from the Event Builder
  ///
  /// @begin_doc_algo
  /// @algo_type{Filter}
  /// @input_banks{REC::Particle}
  /// @output_banks{Filtered; REC::Particle}
  /// @end_doc
  ///
  /// @begin_doc_action
  /// @action_function{EventBuilderFilter::Filter; Scalar}
  /// @end_doc
  ///
  /// @begin_doc_config
  /// @config_param{pids; list[int]; list of PDG codes to filter}
  /// @end_doc
  class EventBuilderFilter : public Algorithm
  {

      DEFINE_IGUANA_ALGORITHM(EventBuilderFilter, clas12::EventBuilderFilter)

    public:

      void Start(hipo::banklist& banks) override;
      void Run(hipo::banklist& banks) const override;
      void Stop() override;

      /// @action_function checks if the PDG `pid` is a part of the list of user-specified PDGs
      /// @param pid the particle PDG to check
      /// @returns `true` if `pid` is one the user wants
      bool Filter(int const pid) const;

    private:

      /// `hipo::banklist` index for the particle bank
      hipo::banklist::size_type b_particle;

      /// Configuration options
      std::set<int> o_pids;
  };
}
