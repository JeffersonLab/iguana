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
      Throw("missing input banks");

    // define the output schemata and banks
    BankMap outBanks = {
      { "particles", std::make_shared<hipo::bank>(inBanks.at("particles")->getSchema()) }
    };

    // filter the input bank for requested PDG code(s)
    std::set<int> acceptedRows;
    for(int row = 0; row < inBanks.at("particles")->getRows(); row++) {
      auto pid    = inBanks.at("particles")->get("pid", row);
      auto accept = m_opt.pids.find(pid) != m_opt.pids.end();
      if(accept) acceptedRows.insert(row);
      m_log->Debug("input PID {} -- accept = {}", pid, accept);
    }

    // fill the output bank
    switch(m_opt.mode) {

      case EventBuilderFilterOptions::Modes::blank:
        {
          outBanks.at("particles")->setRows(inBanks.at("particles")->getRows());
          for(int row = 0; row < inBanks.at("particles")->getRows(); row++) {
            if(acceptedRows.find(row) != acceptedRows.end())
              CopyBankRow(inBanks.at("particles"), row, outBanks.at("particles"), row);
            else
              BlankRow(outBanks.at("particles"), row);
          }
          break;
        }

      case EventBuilderFilterOptions::Modes::compact:
        {
          outBanks.at("particles")->setRows(acceptedRows.size());
          int row = 0;
          for(auto acceptedRow : acceptedRows)
            CopyBankRow(inBanks.at("particles"), acceptedRow, outBanks.at("particles"), row++);
          break;
        }

      default:
        Throw("unknown 'mode' option");

    }

    // dump the banks and return the output
    ShowBanks(inBanks, outBanks);
    return outBanks;
  }


  void EventBuilderFilter::Stop() {
    m_log->Debug("STOP {}", m_name);
  }

}
