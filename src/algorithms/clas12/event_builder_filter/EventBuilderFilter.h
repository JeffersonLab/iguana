#pragma once

#include "services/Algorithm.h"
#include <set>

namespace iguana::clas12 {

  class EventBuilderFilter : public Algorithm {

    public:
      EventBuilderFilter();
      ~EventBuilderFilter() {}

      void Start() override { Algorithm::Start(); }
      void Start(bank_index_cache_t &index_cache) override;
      void Run(bank_vec_t banks) override;
      void Stop() override;

      bool Filter(int pid);

    private:

      /// `bank_vec_t` indices
      int b_particle, b_calo;

      /// configuration options
      std::set<int> o_pids;

  };

}
