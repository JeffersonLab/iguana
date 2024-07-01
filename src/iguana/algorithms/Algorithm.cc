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
  OPTION_TYPE Algorithm::GetOptionScalar(std::string const& key, YAMLReader::node_path_t node_path) const
  {
    CompleteOptionNodePath(key, node_path);
    auto opt = GetCachedOption<OPTION_TYPE>(key);
    if(!opt.has_value()) {
      opt = m_yaml_config->GetScalar<OPTION_TYPE>(node_path);
    }
    if(!opt.has_value()) {
      m_log->Error("Failed to `GetOptionScalar` for key {:?}", key);
      throw std::runtime_error("config file parsing issue");
    }
    PrintOptionValue(key, opt.value());
    return opt.value();
  }
  template int Algorithm::GetOptionScalar(std::string const& key, YAMLReader::node_path_t node_path) const;
  template double Algorithm::GetOptionScalar(std::string const& key, YAMLReader::node_path_t node_path) const;
  template std::string Algorithm::GetOptionScalar(std::string const& key, YAMLReader::node_path_t node_path) const;

  ///////////////////////////////////////////////////////////////////////////////

  template <typename OPTION_TYPE>
  std::vector<OPTION_TYPE> Algorithm::GetOptionVector(std::string const& key, YAMLReader::node_path_t node_path) const
  {
    CompleteOptionNodePath(key, node_path);
    auto opt = GetCachedOption<std::vector<OPTION_TYPE>>(key);
    if(!opt.has_value()) {
      opt = m_yaml_config->GetVector<OPTION_TYPE>(node_path);
    }
    if(!opt.has_value()) {
      m_log->Error("Failed to `GetOptionVector` for key {:?}", key);
      throw std::runtime_error("config file parsing issue");
    }
    PrintOptionValue(key, opt.value());
    return opt.value();
  }
  template std::vector<int> Algorithm::GetOptionVector(std::string const& key, YAMLReader::node_path_t node_path) const;
  template std::vector<double> Algorithm::GetOptionVector(std::string const& key, YAMLReader::node_path_t node_path) const;
  template std::vector<std::string> Algorithm::GetOptionVector(std::string const& key, YAMLReader::node_path_t node_path) const;

  ///////////////////////////////////////////////////////////////////////////////

  template <typename OPTION_TYPE>
  std::set<OPTION_TYPE> Algorithm::GetOptionSet(std::string const& key, YAMLReader::node_path_t node_path) const
  {
    auto val_vec = GetOptionVector<OPTION_TYPE>(key, node_path);
    std::set<OPTION_TYPE> val_set;
    std::copy(val_vec.begin(), val_vec.end(), std::inserter(val_set, val_set.end()));
    return val_set;
  }
  template std::set<int> Algorithm::GetOptionSet(std::string const& key, YAMLReader::node_path_t node_path) const;
  template std::set<double> Algorithm::GetOptionSet(std::string const& key, YAMLReader::node_path_t node_path) const;
  template std::set<std::string> Algorithm::GetOptionSet(std::string const& key, YAMLReader::node_path_t node_path) const;

  ///////////////////////////////////////////////////////////////////////////////

  void Algorithm::SetName(std::string_view name)
  {
    Object::SetName(name);
    if(m_yaml_config)
      m_yaml_config->SetName("config|" + m_name);
  }

  ///////////////////////////////////////////////////////////////////////////////

  std::unique_ptr<YAMLReader> const& Algorithm::GetConfig() const
  {
    return m_yaml_config;
  }

  ///////////////////////////////////////////////////////////////////////////////

  void Algorithm::SetConfig(std::unique_ptr<YAMLReader>&& yaml_config)
  {
    m_yaml_config = std::move(yaml_config);
  }

  ///////////////////////////////////////////////////////////////////////////////

  void Algorithm::SetConfigFile(std::string const& name)
  {
    o_user_config_file = SetOption("config_file", name);
  }

  ///////////////////////////////////////////////////////////////////////////////

  void Algorithm::SetConfigDirectory(std::string const& name)
  {
    o_user_config_dir = SetOption("config_dir", name);
  }

  ///////////////////////////////////////////////////////////////////////////////

  std::string Algorithm::GetDataFile(std::string const& name)
  {
    if(!m_datafile_reader) {
      m_datafile_reader = std::make_unique<DataFileReader>(ConfigFileReader::ConvertAlgoNameToConfigDir(m_class_name), "data|" + m_name);
      m_datafile_reader->SetLogLevel(m_log->GetLevel());
    }
    return m_datafile_reader->FindFile(name);
  }

  ///////////////////////////////////////////////////////////////////////////////

  void Algorithm::ParseYAMLConfig()
  {

    // start YAMLReader instance, if not yet started
    if(!m_yaml_config) {
      // set config files and directories specified by `::SetConfigFile`, `::SetConfigDirectory`, etc.
      o_user_config_file = GetCachedOption<std::string>("config_file").value_or("");
      o_user_config_dir  = GetCachedOption<std::string>("config_dir").value_or("");
      m_log->Debug("Instantiating `YAMLReader`");
      m_yaml_config = std::make_unique<YAMLReader>("config|" + m_name);
      m_yaml_config->SetLogLevel(m_log->GetLevel()); // synchronize log levels
      m_yaml_config->AddDirectory(o_user_config_dir);
      m_yaml_config->AddFile(m_default_config_file);
      m_yaml_config->AddFile(o_user_config_file);
    }
    else
      m_log->Debug("`YAMLReader` already instantiated for this algorithm; using that");

    // parse the files
    m_yaml_config->LoadFiles();

    // if "log" was not set by `SetOption` (i.e., not in `m_option_cache`)
    // - NB: not using `GetCachedOption<T>` here, since `T` can be a few different types for key=='log'
    if(m_option_cache.find("log") == m_option_cache.end()) {
      // check if 'log' is set in the YAML node for this algorithm
      auto log_level_from_yaml = m_yaml_config->GetScalar<std::string>({m_class_name, "log"});
      if(log_level_from_yaml) {
        m_log->SetLevel(log_level_from_yaml.value());
        m_yaml_config->SetLogLevel(log_level_from_yaml.value());
      }
    }
  }

  ///////////////////////////////////////////////////////////////////////////////

  hipo::banklist::size_type Algorithm::GetBankIndex(hipo::banklist& banks, std::string const& bank_name) const
  {
    if(m_rows_only)
      return 0;
    try {
      auto idx = hipo::getBanklistIndex(banks, bank_name);
      m_log->Debug("cached index of bank '{}' is {}", bank_name, idx);
      return idx;
    } catch(std::runtime_error const& ex) {
      m_log->Error("required input bank '{}' not found; cannot `Start` algorithm '{}'", bank_name, m_class_name);
      auto creators = AlgorithmFactory::QueryNewBank(bank_name);
      if(creators)
        m_log->Error(" -> this bank is created by algorithm(s) [{}]; please `Start` ONE of them BEFORE this algorithm", fmt::join(creators.value(), ", "));
      throw std::runtime_error("cannot cache bank index");
    }
  }

  ///////////////////////////////////////////////////////////////////////////////

  void Algorithm::PrintOptionValue(std::string const& key, int const& val, Logger::Level const level, std::string_view prefix) const
  {
    m_log->Print(level, "{}: {:>20} = {} [int]", prefix, key, val);
  }

  void Algorithm::PrintOptionValue(std::string const& key, double const& val, Logger::Level const level, std::string_view prefix) const
  {
    m_log->Print(level, "{}: {:>20} = {} [double]", prefix, key, val);
  }

  void Algorithm::PrintOptionValue(std::string const& key, std::string const& val, Logger::Level const level, std::string_view prefix) const
  {
    m_log->Print(level, "{}: {:>20} = {:?} [string]", prefix, key, val);
  }

  void Algorithm::PrintOptionValue(std::string const& key, std::vector<int> const& val, Logger::Level const level, std::string_view prefix) const
  {
    m_log->Print(level, "{}: {:>20} = ({}) [std::vector<int>]", prefix, key, fmt::join(val, ", "));
  }

  void Algorithm::PrintOptionValue(std::string const& key, std::vector<double> const& val, Logger::Level const level, std::string_view prefix) const
  {
    m_log->Print(level, "{}: {:>20} = ({}) [std::vector<double>]", prefix, key, fmt::join(val, ", "));
  }

  void Algorithm::PrintOptionValue(std::string const& key, std::vector<std::string> const& val, Logger::Level const level, std::string_view prefix) const
  {
    std::vector<std::string> val_quoted;
    for(auto const& s : val)
      val_quoted.push_back(fmt::format("{:?}", s));
    m_log->Print(level, "{}: {:>20} = ({}) [std::vector<string>]", prefix, key, fmt::join(val_quoted, ", "));
  }

  ///////////////////////////////////////////////////////////////////////////////

  hipo::bank& Algorithm::GetBank(hipo::banklist& banks, hipo::banklist::size_type const idx, std::string const& expected_bank_name) const
  {
    if(m_rows_only) {
      m_log->Error("algorithm is in 'rows only' mode; cannot call `Run` since banks are not cached; use action function(s) instead");
    }
    else {
      try {
        auto& result = banks.at(idx);
        if(!expected_bank_name.empty() && result.getSchema().getName() != expected_bank_name)
          m_log->Error("expected input bank '{}' at index={}; got bank named '{}'", expected_bank_name, idx, result.getSchema().getName());
        else
          return result;
      }
      catch(std::out_of_range const& o) {
        m_log->Error("required input bank '{}' not found; cannot `Run` algorithm '{}'", expected_bank_name, m_class_name);
        auto creators = AlgorithmFactory::QueryNewBank(expected_bank_name);
        if(creators)
          m_log->Error(" -> this bank is created by algorithm(s) [{}]; please `Run` ONE of them BEFORE this algorithm", fmt::join(creators.value(), ", "));
      }
    }
    throw std::runtime_error("GetBank failed");
  }

  ///////////////////////////////////////////////////////////////////////////////

  hipo::schema Algorithm::CreateBank(
      hipo::banklist& banks,
      hipo::banklist::size_type& bank_idx,
      std::string const& bank_name,
      std::vector<std::string> schema_def,
      int group_id,
      int item_id) const
  {
    if(!AlgorithmFactory::QueryNewBank(bank_name)) {
      m_log->Error("{:?} creates bank {:?}, which is not registered; new banks must be included in `REGISTER_IGUANA_ALGORITHM` arguments", m_class_name, bank_name);
      throw std::runtime_error("CreateBank failed");
    }
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
    return bank_schema;
  }

  ///////////////////////////////////////////////////////////////////////////////

  void Algorithm::ShowBanks(hipo::banklist& banks, std::string_view message, Logger::Level const level) const
  {
    if(m_log->GetLevel() <= level) {
      if(!message.empty())
        m_log->Print(level, message);
      for(auto& bank : banks)
        bank.show();
    }
  }

  ///////////////////////////////////////////////////////////////////////////////

  void Algorithm::ShowBank(hipo::bank& bank, std::string_view message, Logger::Level const level) const
  {
    if(m_log->GetLevel() <= level) {
      if(!message.empty())
        m_log->Print(level, message);
      bank.show();
    }
  }

  ///////////////////////////////////////////////////////////////////////////////

  template <typename OPTION_TYPE>
  std::optional<OPTION_TYPE> Algorithm::GetCachedOption(std::string const& key) const
  {
    if(key == "")
      return {};
    if(auto it{m_option_cache.find(key)}; it != m_option_cache.end()) {
      try { // get the expected type
        return std::get<OPTION_TYPE>(it->second);
      }
      catch(std::bad_variant_access const& ex) {
        auto printer = [&key, this](auto const& v) {
          m_log->Error("wrong type used in SetOption call for option {:?}; using its default value instead", key);
          PrintOptionValue(key, v, Logger::Level::error, "  USER");
          if(m_log->GetLevel() > Logger::Level::debug)
            m_log->Error("to see the actual option values used (and their types), set the log level to 'debug' or lower");
        };
        std::visit(printer, it->second);
      }
    }
    return {};
  }
  template std::optional<int> Algorithm::GetCachedOption(std::string const& key) const;
  template std::optional<double> Algorithm::GetCachedOption(std::string const& key) const;
  template std::optional<std::string> Algorithm::GetCachedOption(std::string const& key) const;
  template std::optional<std::vector<int>> Algorithm::GetCachedOption(std::string const& key) const;
  template std::optional<std::vector<double>> Algorithm::GetCachedOption(std::string const& key) const;
  template std::optional<std::vector<std::string>> Algorithm::GetCachedOption(std::string const& key) const;

  ///////////////////////////////////////////////////////////////////////////////

  void Algorithm::CompleteOptionNodePath(std::string const& key, YAMLReader::node_path_t& node_path) const
  {
    if(node_path.empty())
      node_path.push_front(key);
    node_path.push_front(m_class_name);
  }
}
