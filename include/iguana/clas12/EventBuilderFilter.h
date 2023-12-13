#pragma once

#include "iguana/Algorithm.h"
#include "iguana/AlgorithmFactory.h"

namespace iguana::clas12 {

  /// @brief Filter the `REC::Particle` (or similar) bank by PID from the Event Builder
  class EventBuilderFilter : public Algorithm {

    public:

      /// @see `Algorithm::Algorithm`
      EventBuilderFilter(std::string name="") : Algorithm(name=="" ? ClassName() : name) {}
      ~EventBuilderFilter() {}
      /// @returns An instance of this algorithm. This is used by `AlgorithmFactory`.
      static algo_t Creator() { return std::make_unique<EventBuilderFilter>(); }
      /// @returns This algorithm's class name
      static std::string ClassName() { return "clas12::EventBuilderFilter"; }

      void Start(hipo::banklist& banks) override;
      void Run(hipo::banklist& banks) const override;
      void Stop() override;

      /// **Action function**: checks if the PDG `pid` is a part of the list of user-specified PDGs
      /// @param pid the particle PDG to check
      /// @returns `true` if `pid` is one the user wants
      bool Filter(const int pid) const;

    private:

      /// True if this algorithm is registered in `AlgorithmFactory`
      static bool s_registered;

      /// `hipo::banklist` index for the particle bank
      hipo::banklist::size_type b_particle, b_calo; // TODO: remove calorimeter

      /// Configuration options
      std::set<int> o_pids;
      int o_testInt; // TODO: remove
      double o_testFloat; // TODO: remove

  };

}
