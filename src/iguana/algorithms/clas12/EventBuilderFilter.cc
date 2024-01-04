#include "EventBuilderFilter.h"

namespace iguana::clas12 {

  REGISTER_ALGORITHM(EventBuilderFilter);

  START_ALGORITHM(EventBuilderFilter) {

    // define options, their default values, and cache them
    CacheOptionToSet("pids", {11, 211}, o_pids);
    CacheOption("testInt", 8, o_testInt); // TODO: remove
    CacheOption("testFloat", 7.0, o_testFloat); // TODO: remove

    // cache expected bank indices
    CacheBankIndex(banks, "REC::Particle", b_particle);
    CacheBankIndex(banks, "REC::Calorimeter", b_calo); // TODO: remove

  }


  RUN_ALGORITHM(EventBuilderFilter) {

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


  bool EventBuilderFilter::Filter(const int pid) const {
    return o_pids.find(pid) != o_pids.end();
  }


  STOP_ALGORITHM(EventBuilderFilter) {
    m_log->Info("test info");
    m_log->Warn("test warn");
    m_log->Error("test error");
  }

}
