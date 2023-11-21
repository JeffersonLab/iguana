#include "EventBuilderFilter.h"

namespace iguana::clas12 {

  void EventBuilderFilter::Start(std::unordered_map<std::string, int> bankVecOrder) {
    m_log->Debug("START {}", m_name);

    // check for input banks
    try {
      b_particle = bankVecOrder.at("REC::Particle");
      b_calo     = bankVecOrder.at("REC::Calorimeter");
    } catch(const std::out_of_range &o) {
      m_log->Error("missing input banks");
      return;
    }

    // set configuration
    m_log->SetLevel(Logger::Level::trace);
    m_opt.pids = {11, 211, -211};
  }


  void EventBuilderFilter::Run(Algorithm::BankVec inBanks) {
    m_log->Debug("RUN {}", m_name);

    // check the input banks existence
    std::shared_ptr<hipo::bank> particleBank;
    std::shared_ptr<hipo::bank> caloBank;
    try {
      particleBank = inBanks.at(b_particle);
      caloBank     = inBanks.at(b_calo);
    } catch(const std::out_of_range &o) {
      m_log->Error("missing input banks");
      return;
    }

    // check these are the correct banks // TODO: maybe too strict
    if(particleBank->getSchema().getName() != "REC::Particle") {
      m_log->Error("bad particle bank");
      return;
    }
    // if(caloBank->getSchema().getName() != "REC::Calorimeter") {
    //   m_log->Error("bad calorimeter bank");
    //   return;
    // }

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
