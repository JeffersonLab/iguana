#include "EventBuilderFilter.h"

namespace iguana::clas12 {

  void EventBuilderFilter::Start() {
    m_log->Debug("START {}", m_name);

    // set configuration
    m_log->SetLevel(Logger::Level::trace);
    m_opt.mode = EventBuilderFilterOptions::Modes::blank;
    m_opt.pids = {11, 211, -211};
  }


  Algorithm::BankMap EventBuilderFilter::Run(Algorithm::BankMap inBanks) {
    m_log->Debug("RUN {}", m_name);

    // check the input banks existence
    if(MissingInputBanks(inBanks, {"particles"}))
      ThrowRun();

    // define the output schemata and banks
    BankMap outBanks = {
      { "particles", hipo::bank(inBanks.at("particles").getSchema()) }
    };

    // set number of output rows
    switch(m_opt.mode) {
      case EventBuilderFilterOptions::Modes::blank:
        outBanks.at("particles").setRows(inBanks.at("particles").getRows());
        break;
      case EventBuilderFilterOptions::Modes::compact:
        outBanks.at("particles").setRows(inBanks.at("particles").getRows()); // FIXME
        break;
    }

    // filter the input bank for requested PDG code(s)
    int outRow = -1;
    for(int inRow = 0; inRow < inBanks.at("particles").getRows(); inRow++) {
      auto inPid = inBanks.at("particles").get("pid",inRow);

      if(m_opt.pids.contains(inPid)) {
        m_log->Debug("input PID {} -- accept", inPid);
        CopyBankRow(
            inBanks.at("particles"),
            outBanks.at("particles"),
            m_opt.mode == EventBuilderFilterOptions::Modes::blank ? inRow : outRow++
            );
      }

      else {
        m_log->Debug("input PID {} -- reject", inPid);
        if(m_opt.mode == EventBuilderFilterOptions::blank)
          BlankRow(outBanks.at("particles"), inRow);
      }

    }

    // dump the banks and return the output
    ShowBanks(inBanks, outBanks);
    return outBanks;
  }


  void EventBuilderFilter::Stop() {
    m_log->Debug("STOP {}", m_name);
  }

}
