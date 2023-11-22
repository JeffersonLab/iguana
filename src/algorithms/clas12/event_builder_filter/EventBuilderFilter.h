#pragma once

#include "services/Algorithm.h"
#include <set>

namespace iguana::clas12 {

  class EventBuilderFilterOptions {
    public:
      std::set<int> pids = {11, 211};
  };


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
      EventBuilderFilterOptions m_opt;
      int b_particle, b_calo;

  };

}
