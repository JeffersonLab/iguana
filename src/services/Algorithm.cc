#include "Algorithm.h"

namespace iguana {

  void Algorithm::SetOption(const std::string key, const option_t val) {
    m_opt[key] = val;
    m_log->Debug("User set option '{}' = {}", key, PrintOptionValue(key));
  }

  void Algorithm::CacheBankIndex(hipo::banklist& banks, const std::string bankName, int& idx) const {
    auto it = std::find_if(
        banks.begin(),
        banks.end(),
        [&bankName] (auto& bank) { return bank.getSchema().getName() == bankName; }
        );
    if(it == banks.end()) {
      m_log->Error("required input bank '{}' not found; cannot `Start` algorithm '{}'", bankName, m_name);
      throw std::runtime_error("cannot cache bank index");
    }
    idx = std::distance(banks.begin(), it);
    m_log->Debug("cached index of bank '{}' is {}", bankName, idx);
  }

  std::string Algorithm::PrintOptionValue(const std::string key) const {
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

  hipo::bank& Algorithm::GetBank(hipo::banklist& banks, const int idx, const std::string expectedBankName) const {
    try {
      auto& result = banks.at(idx);
      if(expectedBankName != "" && result.getSchema().getName() != expectedBankName)
        m_log->Error("expected input bank '{}' at index={}; got bank named '{}'", expectedBankName, idx, result.getSchema().getName());
      else
        return result;
    } catch(const std::out_of_range& o) {
      m_log->Error("required input bank '{}' not found; cannot `Run` algorithm '{}'", expectedBankName, m_name);
    }
    throw std::runtime_error("GetBank failed");
  }

  void Algorithm::MaskRow(hipo::bank& bank, const int row) const {
    // TODO: need https://github.com/gavalian/hipo/issues/35
    // until then, just set the PID to -1
    bank.putInt("pid", row, -1);
  }

  void Algorithm::ShowBanks(hipo::banklist& banks, const std::string message, const Logger::Level level) const {
    if(m_log->GetLevel() <= level) {
      if(message != "")
        m_log->Print(level, message);
      for(auto& bank : banks)
        bank.show();
    }
  }

  void Algorithm::ShowBank(hipo::bank& bank, const std::string message, const Logger::Level level) const {
    if(m_log->GetLevel() <= level) {
      if(message != "")
        m_log->Print(level, message);
      bank.show();
    }
  }

}
