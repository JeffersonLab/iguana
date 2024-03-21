#include "Algorithm.h"

#include <numeric>

namespace iguana {

  void Algorithm::Start()
  {
    m_rows_only             = true;
    hipo::banklist no_banks = {};
    Start(no_banks);
  }

  ///////////////////////////////////////////////////////////////////////////////

  template <typename OPTION_TYPE>
  OPTION_TYPE Algorithm::GetOptionScalar(const std::string key, YAMLReader::node_path_t node_path)
  {
    try {
      CompleteOptionNodePath(key, node_path);
      auto opt = GetCachedOption<OPTION_TYPE>(key);
      auto val = opt ? opt.value() : m_yaml_config->GetScalar<OPTION_TYPE>(node_path);
      if(key != "") {
        m_option_cache[key] = val;
        m_log->Debug("CACHED OPTION: {:>20} = {}", key, PrintOptionValue(key));
      }
      return val;
    }
    catch(const std::runtime_error& ex) {
      m_log->Error("Failed to `GetOptionScalar` for key '{}'", key);
      throw std::runtime_error("config file parsing issue");
    }
  }
  template int Algorithm::GetOptionScalar(const std::string key, YAMLReader::node_path_t node_path);
  template double Algorithm::GetOptionScalar(const std::string key, YAMLReader::node_path_t node_path);
  template std::string Algorithm::GetOptionScalar(const std::string key, YAMLReader::node_path_t node_path);

  ///////////////////////////////////////////////////////////////////////////////

  template <typename OPTION_TYPE>
  std::vector<OPTION_TYPE> Algorithm::GetOptionVector(const std::string key, YAMLReader::node_path_t node_path)
  {
    try {
      CompleteOptionNodePath(key, node_path);
      auto opt = GetCachedOption<std::vector<OPTION_TYPE>>(key);
      auto val = opt ? opt.value() : m_yaml_config->GetVector<OPTION_TYPE>(node_path);
      if(key != "") {
        m_option_cache[key] = val;
        m_log->Debug("CACHED OPTION: {:>20} = {}", key, PrintOptionValue(key));
      }
      return val;
    }
    catch(const std::runtime_error& ex) {
      m_log->Error("Failed to `GetOptionVector` for key '{}'", key);
      throw std::runtime_error("config file parsing issue");
    }
  }
  template std::vector<int> Algorithm::GetOptionVector(const std::string key, YAMLReader::node_path_t node_path);
  template std::vector<double> Algorithm::GetOptionVector(const std::string key, YAMLReader::node_path_t node_path);
  template std::vector<std::string> Algorithm::GetOptionVector(const std::string key, YAMLReader::node_path_t node_path);

  ///////////////////////////////////////////////////////////////////////////////

  template <typename OPTION_TYPE>
  std::set<OPTION_TYPE> Algorithm::GetOptionSet(const std::string key, YAMLReader::node_path_t node_path)
  {
    auto val_vec = GetOptionVector<OPTION_TYPE>(key, node_path);
    std::set<OPTION_TYPE> val_set;
    std::copy(val_vec.begin(), val_vec.end(), std::inserter(val_set, val_set.end()));
    return val_set;
  }
  template std::set<int> Algorithm::GetOptionSet(const std::string key, YAMLReader::node_path_t node_path);
  template std::set<double> Algorithm::GetOptionSet(const std::string key, YAMLReader::node_path_t node_path);
  template std::set<std::string> Algorithm::GetOptionSet(const std::string key, YAMLReader::node_path_t node_path);

  ///////////////////////////////////////////////////////////////////////////////

  void Algorithm::SetName(const std::string name)
  {
    Object::SetName(name);
    if(m_yaml_config)
      m_yaml_config->SetName("config|" + m_name);
  }

  ///////////////////////////////////////////////////////////////////////////////

  std::unique_ptr<YAMLReader>& Algorithm::GetConfig()
  {
    return m_yaml_config;
  }

  ///////////////////////////////////////////////////////////////////////////////

  void Algorithm::SetConfig(std::unique_ptr<YAMLReader>&& yaml_config)
  {
    m_yaml_config = std::move(yaml_config);
  }

  ///////////////////////////////////////////////////////////////////////////////

  void Algorithm::SetConfigFile(std::string name)
  {
    o_user_config_file = SetOption("config_file", name);
  }

  ///////////////////////////////////////////////////////////////////////////////

  void Algorithm::SetConfigDirectory(std::string name)
  {
    o_user_config_dir = SetOption("config_dir", name);
  }

  ///////////////////////////////////////////////////////////////////////////////

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

  ///////////////////////////////////////////////////////////////////////////////

  hipo::banklist::size_type Algorithm::GetBankIndex(hipo::banklist& banks, const std::string bank_name) const
  {
    if(m_rows_only)
      return 0;
    auto it = std::find_if(
        banks.begin(),
        banks.end(),
        [&bank_name](auto& bank)
        { return bank.getSchema().getName() == bank_name; });
    if(it == banks.end()) {
      m_log->Error("required input bank '{}' not found; cannot `Start` algorithm '{}'", bank_name, m_class_name);
      auto creators = AlgorithmFactory::QueryNewBank(bank_name);
      if(creators)
        m_log->Error(" -> this bank is created by algorithm(s) [{}]; please `Start` ONE of them BEFORE this algorithm", fmt::join(creators.value(), ", "));
      throw std::runtime_error("cannot cache bank index");
    }
    auto idx = std::distance(banks.begin(), it);
    m_log->Debug("cached index of bank '{}' is {}", bank_name, idx);
    return idx;
  }

  ///////////////////////////////////////////////////////////////////////////////

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

  ///////////////////////////////////////////////////////////////////////////////

  hipo::bank& Algorithm::GetBank(hipo::banklist& banks, const hipo::banklist::size_type idx, const std::string expected_bank_name) const
  {
    if(m_rows_only) {
      m_log->Error("algorithm is in 'rows only' mode; cannot call `Run` since banks are not cached; use action function(s) instead");
    }
    else {
      try {
        auto& result = banks.at(idx);
        if(expected_bank_name != "" && result.getSchema().getName() != expected_bank_name)
          m_log->Error("expected input bank '{}' at index={}; got bank named '{}'", expected_bank_name, idx, result.getSchema().getName());
        else
          return result;
      }
      catch(const std::out_of_range& o) {
        m_log->Error("required input bank '{}' not found; cannot `Run` algorithm '{}'", expected_bank_name, m_class_name);
        auto creators = AlgorithmFactory::QueryNewBank(expected_bank_name);
        if(creators)
          m_log->Error(" -> this bank is created by algorithm(s) [{}]; please `Run` ONE of them BEFORE this algorithm", fmt::join(creators.value(), ", "));
      }
    }
    throw std::runtime_error("GetBank failed");
  }

  ///////////////////////////////////////////////////////////////////////////////

  void Algorithm::MaskRow(hipo::bank& bank, const int row) const
  {
    // TODO: need https://github.com/gavalian/hipo/issues/35
    // until then, just set the PID to -1
    bank.putInt("pid", row, -1);
  }

  ///////////////////////////////////////////////////////////////////////////////

  hipo::schema Algorithm::CreateBank(
      hipo::banklist& banks,
      hipo::banklist::size_type& bank_idx,
      std::string bank_name,
      std::vector<std::string> schema_def,
      int group_id,
      int item_id) const
  {
    if(schema_def.empty()) {
      m_log->Error("empty schema_def in CreateBank");
      throw std::runtime_error("CreateBank failed");
    }
    hipo::schema bank_schema(bank_name.c_str(), group_id, item_id);
    bank_schema.parse(std::accumulate(
        std::next(schema_def.begin()),
        schema_def.end(),
        schema_def[0],
        [](std::string a, std::string b)
        { return a + "," + b; }));
    banks.push_back({bank_schema});
    bank_idx = GetBankIndex(banks, bank_name);
    if(!AlgorithmFactory::QueryNewBank(bank_name))
      m_log->Error("'{}' is not registered as a creator algorithm; `REGISTER_IGUANA_NEW_BANKS` must be called in the algorithm source code", m_class_name);
    return bank_schema;
  }

  ///////////////////////////////////////////////////////////////////////////////

  void Algorithm::ShowBanks(hipo::banklist& banks, const std::string message, const Logger::Level level) const
  {
    if(m_log->GetLevel() <= level) {
      if(message != "")
        m_log->Print(level, message);
      for(auto& bank : banks)
        bank.show();
    }
  }

  ///////////////////////////////////////////////////////////////////////////////

  void Algorithm::ShowBank(hipo::bank& bank, const std::string message, const Logger::Level level) const
  {
    if(m_log->GetLevel() <= level) {
      if(message != "")
        m_log->Print(level, message);
      bank.show();
    }
  }

  ///////////////////////////////////////////////////////////////////////////////

  template <typename OPTION_TYPE>
  std::optional<OPTION_TYPE> Algorithm::GetCachedOption(const std::string key) const
  {
    if(key == "")
      return {};
    if(auto it{m_option_cache.find(key)}; it != m_option_cache.end()) {
      try { // get the expected type
        return std::get<OPTION_TYPE>(it->second);
      }
      catch(const std::bad_variant_access& ex) {
        m_log->Warn("user called SetOption for option '{}' and set it to '{}', which is the wrong type; IGNORING", key, PrintOptionValue(key));
      }
    }
    return {};
  }
  template std::optional<int> Algorithm::GetCachedOption(const std::string key) const;
  template std::optional<double> Algorithm::GetCachedOption(const std::string key) const;
  template std::optional<std::string> Algorithm::GetCachedOption(const std::string key) const;
  template std::optional<std::vector<int>> Algorithm::GetCachedOption(const std::string key) const;
  template std::optional<std::vector<double>> Algorithm::GetCachedOption(const std::string key) const;
  template std::optional<std::vector<std::string>> Algorithm::GetCachedOption(const std::string key) const;

  ///////////////////////////////////////////////////////////////////////////////

  void Algorithm::CompleteOptionNodePath(const std::string key, YAMLReader::node_path_t& node_path) const
  {
    if(node_path.empty())
      node_path.push_front(key);
    node_path.push_front(m_class_name);
  }
}
