#include "EventBuilderFilter.h"

namespace iguana::clas12 {

  EventBuilderFilter::EventBuilderFilter() : Algorithm("event_builder_filter") {
    m_requiredBanks = {
      "REC::Particle",
      "REC::Calorimeter"
    };
  }

  void EventBuilderFilter::Start(bank_index_cache_t &index_cache) {

    // set configuration
    m_log->SetLevel(Logger::Level::trace);
    m_log->Debug("START {}", m_name);
    m_opt.pids = {11, 211, -211};

    // cache expected bank indices
    CacheBankIndex(index_cache, b_particle, "REC::Particle");
    CacheBankIndex(index_cache, b_calo,     "REC::Calorimeter");
    m_log->Error("{} {}", b_particle, b_calo);

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
      auto accept = m_opt.pids.find(pid) != m_opt.pids.end();
      if(!accept)
        BlankRow(particleBank, row);
      m_log->Debug("input PID {} -- accept = {}", pid, accept);
    }

    // dump the modified bank
    ShowBank(particleBank, Logger::Header("OUTPUT PARTICLES"));
  }


  void EventBuilderFilter::Stop() {
    m_log->Debug("STOP {}", m_name);
  }

}
