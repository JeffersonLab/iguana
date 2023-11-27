#include "EventBuilderFilter.h"

namespace iguana::clas12 {

  EventBuilderFilter::EventBuilderFilter() : Algorithm("event_builder_filter") {
    // define required banks
    m_requiredBanks = { "REC::Particle", "REC::Calorimeter" };
  }

  void EventBuilderFilter::Start(bank_index_cache_t &index_cache) {

    // set logger
    // TODO: should be done by configuration
    m_log->SetLevel(Logger::Level::trace);
    m_log->Debug("START {}", m_name);

    // cache options and define their defaults
    CacheOption("pids", std::set<int>{11, 211}, o_pids);

    // cache expected bank indices
    CacheBankIndex(index_cache, b_particle, "REC::Particle");
    CacheBankIndex(index_cache, b_calo,     "REC::Calorimeter");

  }


  void EventBuilderFilter::Run(bank_vec_t banks) {
    m_log->Debug("RUN {}", m_name);

    // get the banks
    auto particleBank = GetBank(banks, b_particle, "REC::Particle");
    auto caloBank     = GetBank(banks, b_calo,     "REC::Calorimeter");

    // dump the bank
    ShowBank(particleBank, Logger::Header("INPUT PARTICLES"));

    // filter the input bank for requested PDG code(s)
    for(int row = 0; row < particleBank->getRows(); row++) {
      auto pid    = particleBank->get("pid", row);
      auto accept = Filter(pid);
      if(!accept)
        BlankRow(particleBank, row);
      m_log->Debug("input PID {} -- accept = {}", pid, accept);
    }

    // dump the modified bank
    ShowBank(particleBank, Logger::Header("OUTPUT PARTICLES"));
  }


  bool EventBuilderFilter::Filter(int pid) {
    return o_pids.find(pid) != o_pids.end();
  }


  void EventBuilderFilter::Stop() {
    m_log->Debug("STOP {}", m_name);
  }

}
