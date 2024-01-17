#include "EventBuilderFilter.h"

namespace iguana::clas12 {

  REGISTER_IGUANA_ALGORITHM(EventBuilderFilter);

  void EventBuilderFilter::Start(hipo::banklist& banks) {

    // define options, their default values, and cache them
    CacheOptionToSet("pids", {11, 211}, o_pids);

    // cache expected bank indices
    CacheBankIndex(banks, "REC::Particle", b_particle);

  }


  void EventBuilderFilter::Run(hipo::banklist& banks) const {

    // get the banks
    auto& particleBank = GetBank(banks, b_particle, "REC::Particle");

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


  void EventBuilderFilter::Stop() {
  }

}
