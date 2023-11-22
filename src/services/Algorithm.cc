#include "Algorithm.h"

namespace iguana {

  Algorithm::Algorithm(std::string name) : m_name(name) {
    m_log = std::make_shared<Logger>(m_name);
  }

  void Algorithm::Start() {
    bank_index_cache_t index_cache;
    int i = 0;
    for(auto requiredBank : m_requiredBanks)
      index_cache.insert({requiredBank, i++});
    Start(index_cache);
  }

  void Algorithm::CacheBankIndex(bank_index_cache_t index_cache, int &idx, std::string bankName) {
    try {
      idx = index_cache.at(bankName);
    } catch(const std::out_of_range &o) {
      Throw(fmt::format("required input bank '{}' not found; cannot `Start` algorithm '{}'", bankName, m_name));
    }
    m_log->Debug("cached index of bank '{}' is {}", bankName, idx);
  }

  bank_ptr Algorithm::GetBank(bank_vec_t banks, int idx, std::string expectedBankName) {
    bank_ptr result;
    try {
      result = banks.at(idx);
    } catch(const std::out_of_range &o) {
      Throw(fmt::format("required input bank '{}' not found; cannot `Run` algorithm '{}'", expectedBankName, m_name));
    }
    if(expectedBankName != "" && result->getSchema().getName() != expectedBankName) {
      Throw(fmt::format("expected input bank '{}' at index={}; got bank named '{}'", expectedBankName, idx, result->getSchema().getName()));
    }
    return result;
  }

  void Algorithm::CopyBankRow(bank_ptr srcBank, int srcRow, bank_ptr destBank, int destRow) {
    // TODO: check srcBank->getSchema() == destBank.getSchema()
    for(int item = 0; item < srcBank->getSchema().getEntries(); item++) {
      auto val = srcBank->get(item, srcRow);
      destBank->put(item, destRow, val);
    }
  }

  void Algorithm::BlankRow(bank_ptr bank, int row) {
    for(int item = 0; item < bank->getSchema().getEntries(); item++) {
      bank->put(item, row, 0);
    }
  }

  void Algorithm::ShowBanks(bank_vec_t banks, std::string message, Logger::Level level) {
    if(m_log->GetLevel() <= level) {
      if(message != "")
        m_log->Print(level, message);
      for(auto bank : banks)
        bank->show();
    }
  }

  void Algorithm::ShowBank(bank_ptr bank, std::string message, Logger::Level level) {
    if(m_log->GetLevel() <= level) {
      if(message != "")
        m_log->Print(level, message);
      bank->show();
    }
  }

  void Algorithm::Throw(std::string message) {
    m_log->Error("CRITICAL RUNTIME ERROR!");
    throw std::runtime_error(fmt::format("{}; Algorithm '{}' stopped!", message, m_name));
  }

}
