#include "Algorithm.h"

namespace iguana {

  void Algorithm::Start()
  {
    m_rows_only             = true;
    hipo::banklist no_banks = {};
    Start(no_banks);
  }

  void Algorithm::SetName(const std::string name)
  {
    Object::SetName(name);
    if(m_yaml_config)
      m_yaml_config->SetName("config|" + m_name);
  }

  std::unique_ptr<YAMLReader>& Algorithm::GetConfig()
  {
    return m_yaml_config;
  }

  void Algorithm::SetConfig(std::unique_ptr<YAMLReader>&& yaml_config)
  {
    m_yaml_config = std::move(yaml_config);
  }

  void Algorithm::SetConfigFile(std::string name)
  {
    o_user_config_file = SetOption("config_file", name);
  }

  void Algorithm::SetConfigDirectory(std::string name)
  {
    o_user_config_dir = SetOption("config_dir", name);
  }

  void Algorithm::ParseYAMLConfig()
  {
    if(!m_yaml_config) {
      o_user_config_file = GetCachedOption<std::string>("config_file").value_or("");
      o_user_config_dir  = GetCachedOption<std::string>("config_dir").value_or("");
      m_log->Debug("Instantiating `YAMLReader`");
      m_yaml_config = std::make_unique<YAMLReader>("config|" + m_name);
      m_yaml_config->SetLogLevel(m_log->GetLevel());
      m_yaml_config->AddDirectory(o_user_config_dir);
      m_yaml_config->AddFile(m_default_config_file);
      m_yaml_config->AddFile(o_user_config_file);
    }
    else
      m_log->Debug("`YAMLReader` already instantiated for this algorithm; using that");
    m_yaml_config->LoadFiles();
  }

  hipo::banklist::size_type Algorithm::GetBankIndex(hipo::banklist& banks, const std::string bankName) const
  {
    if(m_rows_only)
      return 0;
    auto it = std::find_if(
        banks.begin(),
        banks.end(),
        [&bankName](auto& bank)
        { return bank.getSchema().getName() == bankName; });
    if(it == banks.end()) {
      m_log->Error("required input bank '{}' not found; cannot `Start` algorithm '{}'", bankName, m_name);
      throw std::runtime_error("cannot cache bank index");
    }
    auto idx = std::distance(banks.begin(), it);
    m_log->Debug("cached index of bank '{}' is {}", bankName, idx);
    return idx;
  }

  std::string Algorithm::PrintOptionValue(const std::string key) const
  {
    if(auto it{m_option_cache.find(key)}; it != m_option_cache.end()) {
      auto val = it->second;
      if(const auto valPtr(std::get_if<int>(&val)); valPtr)
        return fmt::format("{} [{}]", *valPtr, "int");
      else if(const auto valPtr(std::get_if<double>(&val)); valPtr)
        return fmt::format("{} [{}]", *valPtr, "double");
      else if(const auto valPtr(std::get_if<std::string>(&val)); valPtr)
        return fmt::format("{:?} [{}]", *valPtr, "string");
      else if(const auto valPtr(std::get_if<std::vector<int>>(&val)); valPtr)
        return fmt::format("({}) [{}]", fmt::join(*valPtr, ", "), "vector<int>");
      else if(const auto valPtr(std::get_if<std::vector<double>>(&val)); valPtr)
        return fmt::format("({}) [{}]", fmt::join(*valPtr, ", "), "vector<double>");
      else if(const auto valPtr(std::get_if<std::vector<std::string>>(&val)); valPtr) {
        std::vector<std::string> valQuoted;
        for(const auto& s : *valPtr)
          valQuoted.push_back(fmt::format("{:?}", s));
        return fmt::format("({}) [{}]", fmt::join(valQuoted, ", "), "vector<string>");
      }
      else {
        m_log->Error("option '{}' type has no printer defined in Algorithm::PrintOptionValue", key);
        return "UNKNOWN";
      }
    }
    else
      m_log->Error("option '{}' not found by Algorithm::PrintOptionValue", key);
    return "UNKNOWN";
  }

  hipo::bank& Algorithm::GetBank(hipo::banklist& banks, const hipo::banklist::size_type idx, const std::string expectedBankName) const
  {
    if(m_rows_only) {
      m_log->Error("algorithm is in 'rows only' mode; cannot call `Run` since banks are not cached; use action function(s) instead");
    }
    else {
      try {
        auto& result = banks.at(idx);
        if(expectedBankName != "" && result.getSchema().getName() != expectedBankName)
          m_log->Error("expected input bank '{}' at index={}; got bank named '{}'", expectedBankName, idx, result.getSchema().getName());
        else
          return result;
      }
      catch(const std::out_of_range& o) {
        m_log->Error("required input bank '{}' not found; cannot `Run` algorithm '{}'", expectedBankName, m_name);
      }
    }
    throw std::runtime_error("GetBank failed");
  }

  void Algorithm::MaskRow(hipo::bank& bank, const int row) const
  {
    // TODO: need https://github.com/gavalian/hipo/issues/35
    // until then, just set the PID to -1
    bank.putInt("pid", row, -1);
  }

  void Algorithm::ShowBanks(hipo::banklist& banks, const std::string message, const Logger::Level level) const
  {
    if(m_log->GetLevel() <= level) {
      if(message != "")
        m_log->Print(level, message);
      for(auto& bank : banks)
        bank.show();
    }
  }

  void Algorithm::ShowBank(hipo::bank& bank, const std::string message, const Logger::Level level) const
  {
    if(m_log->GetLevel() <= level) {
      if(message != "")
        m_log->Print(level, message);
      bank.show();
    }
  }

}
