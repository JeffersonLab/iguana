#pragma once

#include "services/Algorithm.h"

namespace iguana::clas12 {

  class EventBuilderFilterOptions {
    public:
      enum Modes { blank, compact };
      Modes mode = blank;
      std::set<int> pids = {11, 211};
  };


  class EventBuilderFilter : public Algorithm {

    public:
      EventBuilderFilter() : Algorithm("event_builder_filter") {}
      ~EventBuilderFilter() {}

      void Start() override;
      Algorithm::BankMap Run(Algorithm::BankMap inBanks) override;
      void Stop() override;

    private:
      EventBuilderFilterOptions m_opt;

  };

}
