#pragma once

#include "services/Algorithm.h"

namespace iguana::clas12 {

  class EventBuilderFilterOptions {
    public:
      std::set<int> pids = {11, 211};
  };


  class EventBuilderFilter : public Algorithm {

    public:
      EventBuilderFilter() : Algorithm("event_builder_filter") {
        m_requiredBanks = {
          "REC::Particle",
          "REC::Calorimeter"
        };
      }
      ~EventBuilderFilter() {}

      void Start(std::unordered_map<std::string, int> bankVecOrder) override;
      void Run(Algorithm::BankVec inBanks) override;
      void Stop() override;

    private:
      EventBuilderFilterOptions m_opt;
      int b_particle, b_calo;

  };

}
