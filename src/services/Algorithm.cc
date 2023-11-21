#include "Algorithm.h"

namespace iguana {

  Algorithm::Algorithm(std::string name) : m_name(name) {
    m_log = std::make_shared<Logger>(m_name);
  }

  void Algorithm::Start() {
    std::unordered_map<std::string, int> m;
    int i = 0;
    for(auto requiredBank : m_requiredBanks)
      m.insert({requiredBank, i});
    Start(m);
  }

  void Algorithm::CopyBankRow(std::shared_ptr<hipo::bank> srcBank, int srcRow, std::shared_ptr<hipo::bank> destBank, int destRow) {
    // TODO: check srcBank->getSchema() == destBank.getSchema()
    for(int item = 0; item < srcBank->getSchema().getEntries(); item++) {
      auto val = srcBank->get(item, srcRow);
      destBank->put(item, destRow, val);
    }
  }

  void Algorithm::BlankRow(std::shared_ptr<hipo::bank> bank, int row) {
    for(int item = 0; item < bank->getSchema().getEntries(); item++) {
      bank->put(item, row, 0);
    }
  }

  void Algorithm::ShowBanks(BankVec banks, std::string message, Logger::Level level) {
    if(m_log->GetLevel() <= level) {
      if(message != "")
        m_log->Print(level, message);
      for(auto bank : banks)
        bank->show();
    }
  }

  void Algorithm::ShowBank(std::shared_ptr<hipo::bank> bank, std::string message, Logger::Level level) {
    if(m_log->GetLevel() <= level) {
      if(message != "")
        m_log->Print(level, message);
      bank->show();
    }
  }

  void Algorithm::Throw(std::string message) {
    throw std::runtime_error(fmt::format("CRITICAL ERROR: {}; Algorithm '{}' stopped!", message, m_name));
  }

}
