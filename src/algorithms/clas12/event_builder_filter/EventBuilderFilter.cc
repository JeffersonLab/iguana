#include "EventBuilderFilter.h"

namespace iguana::clas12 {

  void EventBuilderFilter::Start() {
    m_log->Info("start event builder filter");
  }

  Algorithm::BankMap EventBuilderFilter::Run(Algorithm::BankMap inputBanks) {

    // check the input banks existence
    if(MissingInputBanks(inputBanks, {"particles"}))
      ThrowRun();

    // define the output schemata and banks
    BankMap outputBanks = {
      { "particles", hipo::bank(inputBanks.at("particles").getSchema(), inputBanks.at("particles").getRows()) }
    };

    // filter the input bank for requested PDG code(s)
    int j=0;
    for(int i=0; i<inputBanks.at("particles").getRows(); i++) {
      auto pid = inputBanks.at("particles").get("pid",i);
      m_log->Info("INPUT PID = {}", pid);
      if(pid==11)
        outputBanks.at("particles").put("pid", j++, pid);
    }

    // print
    m_log->Info("SHOW OUTPUT BANKS");
    outputBanks.at("particles").show();

    return outputBanks;
  }

  void EventBuilderFilter::Stop() {
    m_log->Info("stop event builder filter");
  }

}
