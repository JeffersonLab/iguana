#include "EventBuilderFilter.h"

namespace iguana::clas12 {

  EventBuilderFilter::EventBuilderFilter() : Algorithm("event_builder_filter") {

    // define required banks
    m_requiredBanks = { "REC::Particle", "REC::Calorimeter" }; // TODO: remove calorimeter

  }

  void EventBuilderFilter::Start(bank_index_cache_t& index_cache) {

    // define options, their default values, and cache them
    CacheOption("pids", std::set<int>{11, 211}, o_pids);
    CacheOption("testInt", 8, o_testInt); // TODO: remove
    CacheOption("testFloat", 7.0, o_testFloat); // TODO: remove

    // cache expected bank indices
    CacheBankIndex(index_cache, b_particle, "REC::Particle");
    CacheBankIndex(index_cache, b_calo,     "REC::Calorimeter"); // TODO: remove

  }


  void EventBuilderFilter::Run(hipo::banklist& banks) {

    // get the banks
    auto& particleBank = GetBank(banks, b_particle, "REC::Particle");
    // auto& caloBank     = GetBank(banks, b_calo,     "REC::Calorimeter"); // TODO: remove

    // dump the bank
    ShowBank(particleBank, Logger::Header("INPUT PARTICLES"));

    // filter the input bank for requested PDG code(s)
    for(int row = 0; row < particleBank.getRows(); row++) {
      auto pid    = particleBank.getInt("pid", row);
      auto accept = Filter(pid);
      if(!accept)
        MaskRow(particleBank, row);
      m_log->Debug("input PID {} -- accept = {}", pid, accept);
    }

    // dump the modified bank
    ShowBank(particleBank, Logger::Header("OUTPUT PARTICLES"));
  }


  bool EventBuilderFilter::Filter(int pid) {
    return o_pids.find(pid) != o_pids.end();
  }


  void EventBuilderFilter::Stop() {
    m_log->Info("test info");
    m_log->Warn("test warn");
    m_log->Error("test error");
  }

}
