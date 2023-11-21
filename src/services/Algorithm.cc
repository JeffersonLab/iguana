#include "Algorithm.h"

namespace iguana {

  Algorithm::Algorithm(std::string name) : m_name(name) {
    m_log = std::make_shared<Logger>(m_name);
  }

  bool Algorithm::MissingInputBanks(BankMap banks, std::set<std::string> keys) {
    for(auto key : keys) {
      if(banks.find(key) == banks.end()) {
        m_log->Error("Algorithm '{}' is missing the input bank '{}'", m_name, key);
        m_log->Error("  => the following input banks are required by '{}':", m_name);
        for(auto k : keys)
          m_log->Error("     - {}", k);
        return true;
      }
    }
    return false;
  }

  void Algorithm::CopyBankRow(hipo::bank srcBank, int srcRow, hipo::bank destBank, int destRow) {
    // TODO: check srcBank.getSchema() == destBank.getSchema()
    for(int item = 0; item < srcBank.getSchema().getEntries(); item++) {
      auto val = srcBank.get(item, srcRow);
      destBank.put(item, destRow, val);
    }
  }

  void Algorithm::BlankRow(hipo::bank bank, int row) {
    for(int item = 0; item < bank.getSchema().getEntries(); item++) {
      bank.put(item, row, 0);
    }
  }

  void Algorithm::ShowBanks(BankMap banks, std::string message, Logger::Level level) {
    if(m_log->GetLevel() <= level) {
      m_log->Print(level, message);
      for(auto [key,bank] : banks) {
        m_log->Print(level, "BANK: '{}'", key);
        bank.show();
      }
    }
  }

  void Algorithm::ShowBanks(BankMap inBanks, BankMap outBanks, Logger::Level level) {
    ShowBanks(inBanks,  "===== INPUT BANKS =====",  level);
    ShowBanks(outBanks, "===== OUTPUT BANKS =====", level);
  }

  void Algorithm::Throw(std::string message) {
    throw std::runtime_error(fmt::format("CRITICAL ERROR: {}; Algorithm '{}' stopped!", message, m_name));
  }

}
