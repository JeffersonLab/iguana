#pragma once

#include "services/Algorithm.h"

namespace iguana::clas12 {

  class EventBuilderFilter : public Algorithm {

    public:
      EventBuilderFilter() : Algorithm("event_builder_filter") {}
      ~EventBuilderFilter() {}

      void Start() override;
      Algorithm::BankMap Run(Algorithm::BankMap inputBanks) override;
      void Stop() override;
  };
}
