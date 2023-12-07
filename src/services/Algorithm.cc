#include "Algorithm.h"

namespace iguana {

  Algorithm::Algorithm(const std::string name) : m_name(name) {
    m_log = std::make_unique<Logger>(m_name);
  }

  void Algorithm::SetOption(const std::string key, const option_value_t val) {
    m_opt[key] = val;
    m_log->Debug("User set option '{}' = {}", key, PrintOptionValue(key));
  }

  std::unique_ptr<Logger>& Algorithm::Log() {
    return m_log;
  }

  void Algorithm::CacheBankIndex(hipo::banklist& banks, int& idx, const std::string bankName) const {
    auto it = std::find_if(
        banks.begin(),
        banks.end(),
        [&bankName] (auto& bank) { return bank.getSchema().getName() == bankName; }
        );
    if(it == banks.end())
      Throw(fmt::format("required input bank '{}' not found; cannot `Start` algorithm '{}'", bankName, m_name));
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
      if(expectedBankName != "" && result.getSchema().getName() != expectedBankName) {
        Throw(fmt::format("expected input bank '{}' at index={}; got bank named '{}'", expectedBankName, idx, result.getSchema().getName()));
      }
      return result;
    } catch(const std::out_of_range& o) {
      Throw(fmt::format("required input bank '{}' not found; cannot `Run` algorithm '{}'", expectedBankName, m_name));
    }
    throw std::runtime_error("GetBank failed"); // avoid `-Wreturn-type` warning
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

  void Algorithm::Throw(const std::string message) const {
    m_log->Error("CRITICAL RUNTIME ERROR!");
    throw std::runtime_error(fmt::format("{}; Algorithm '{}' stopped!", message, m_name));
  }

}
