#pragma once

#include "services/Algorithm.h"
#include <set>

namespace iguana::clas12 {

  class EventBuilderFilter : public Algorithm {

    public:
      EventBuilderFilter();
      ~EventBuilderFilter() {}

      void Start() override { Algorithm::Start(); }
      void Start(const bank_index_cache_t& index_cache) override;
      void Run(hipo::banklist& banks) const override;
      void Stop() override;

      bool Filter(const int pid) const;

    private:

      /// `hipo::banklist` indices
      int b_particle, b_calo; // TODO: remove calorimeter

      /// configuration options
      std::set<int> o_pids;
      int o_testInt; // TODO: remove
      double o_testFloat; // TODO: remove

  };

}
