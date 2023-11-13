#include "EventBuilderFilter.h"

namespace iguana::clas12 {

  void EventBuilderFilter::Start() {
    m_log->Info("start event builder filter");
  }

  Algorithm::BankMap EventBuilderFilter::Run(Algorithm::BankMap inputBanks) {
    return inputBanks;
  }

  void EventBuilderFilter::Stop() {
    m_log->Info("stop event builder filter");
  }

}
