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

      void Start(std::unordered_map<std::string, int> bankVecIndices) override;
      void Run(Algorithm::BankVec banks) override;
      void Stop() override;

    private:
      EventBuilderFilterOptions m_opt;
      int b_particle, b_calo;

  };

}
