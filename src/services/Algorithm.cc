#include "Algorithm.h"

namespace iguana {

  Algorithm::Algorithm(std::string name) : m_name(name) {
    m_log = std::make_unique<Logger>(m_name);
  }

  void Algorithm::Start() {
    bank_index_cache_t index_cache;
    int i = 0;
    for(auto requiredBank : m_requiredBanks)
      index_cache.insert({requiredBank, i++});
    Start(index_cache);
  }

  void Algorithm::SetOption(std::string key, option_value_t val) {
    m_opt[key] = val;
    m_log->Debug("User set option '{}' = {}", key, PrintOptionValue(key));
  }

  std::unique_ptr<Logger>& Algorithm::Log() {
    return m_log;
  }

  void Algorithm::CacheBankIndex(bank_index_cache_t index_cache, int& idx, std::string bankName) {
    try {
      idx = index_cache.at(bankName);
    } catch(const std::out_of_range& o) {
      Throw(fmt::format("required input bank '{}' not found; cannot `Start` algorithm '{}'", bankName, m_name));
    }
    m_log->Debug("cached index of bank '{}' is {}", bankName, idx);
  }

  std::string Algorithm::PrintOptionValue(std::string key) {
    if(auto it{m_opt.find(key)}; it != m_opt.end()) {
      auto val = it->second;
      std::string format_str = "{} [{}]";
      if      (const auto valPtr(std::get_if<int>(&val));           valPtr) return fmt::format("{} [{}]", *valPtr,                 "int");
      else if (const auto valPtr(std::get_if<double>(&val));        valPtr) return fmt::format("{} [{}]", *valPtr,                 "double");
      else if (const auto valPtr(std::get_if<std::string>(&val));   valPtr) return fmt::format("{} [{}]", *valPtr,                 "string");
      else if (const auto valPtr(std::get_if<std::set<int>>(&val)); valPtr) return fmt::format("({}) [{}]", fmt::join(*valPtr,", "), "set<int>");
      else {
        m_log->Error("option '{}' type has no printer defined in Algorithm::PrintOptionValue", key);
        return "UNKNOWN";
      }
    }
    else
      m_log->Error("option '{}' not found by Algorithm::PrintOptionValue", key);
    return "UNKNOWN";
  }

  hipo::bank& Algorithm::GetBank(hipo::banklist& banks, int idx, std::string expectedBankName) {
    try {
      auto& result = banks.at(idx);
      if(expectedBankName != "" && result.getSchema().getName() != expectedBankName) {
        Throw(fmt::format("expected input bank '{}' at index={}; got bank named '{}'", expectedBankName, idx, result.getSchema().getName()));
      }
      return result;
    } catch(const std::out_of_range& o) {
      Throw(fmt::format("required input bank '{}' not found; cannot `Run` algorithm '{}'", expectedBankName, m_name));
    }
    throw std::runtime_error("GetBank failed"); // avoid `-Wreturn-type` warning
  }

  void Algorithm::MaskRow(hipo::bank& bank, int row) {
    // TODO: need https://github.com/gavalian/hipo/issues/35
    // until then, just set the PID to -1
    bank.putInt("pid", row, -1);
  }

  void Algorithm::ShowBanks(hipo::banklist& banks, std::string message, Logger::Level level) {
    if(m_log->GetLevel() <= level) {
      if(message != "")
        m_log->Print(level, message);
      for(auto& bank : banks)
        bank.show();
    }
  }

  //
  // FIXME: const protection should be applied everywhere
  // it's possbile, to indicate immuatibility
  //
  //
  void Algorithm::ShowBank(hipo::bank& bank, std::string message, Logger::Level level) {
    if(m_log->GetLevel() <= level) {
      if(message != "")
        m_log->Print(level, message);
      bank.show();
    }
  }

  void Algorithm::Throw(std::string message) {
    m_log->Error("CRITICAL RUNTIME ERROR!");
    throw std::runtime_error(fmt::format("{}; Algorithm '{}' stopped!", message, m_name));
  }

}
