#include "Algorithm.h"

namespace iguana {

  hipo::banklist::size_type tools::GetBankIndex(
      hipo::banklist& banks,
      std::string const& bank_name,
      unsigned int const& variant) noexcept(false)
  {
    unsigned int num_found = 0;
    for(hipo::banklist::size_type i = 0; i < banks.size(); i++) {
      auto& bank = banks.at(i);
      if(bank.getSchema().getName() == bank_name) {
        if(num_found == variant)
          return i;
        num_found++;
      }
    }
    throw std::runtime_error("GetBankIndex failed to find bank \"" + bank_name + "\"");
  }

  ///////////////////////////////////////////////////////////////////////////////

  void Algorithm::Start(hipo::banklist& banks)
  {
    ParseYAMLConfig();
    m_log->Debug(fmt::format("/{}\\", Logger::Header("ConfigHook")));
    ConfigHook();
    m_log->Debug(fmt::format("\\{:=^50}/", ""));
    m_log->Debug(fmt::format("/{}\\", Logger::Header("StartHook")));
    StartHook(banks);
    m_log->Debug(fmt::format("\\{:=^50}/", ""));
  }

  ///////////////////////////////////////////////////////////////////////////////

  void Algorithm::Start()
  {
    m_rows_only             = true;
    hipo::banklist no_banks = {};
    Start(no_banks);
  }

  ///////////////////////////////////////////////////////////////////////////////

  bool Algorithm::Run(hipo::banklist& banks) const
  {
    m_log->Trace("=========== {}::RunHook ===========", m_class_name);
    return RunHook(banks);
  }

  ///////////////////////////////////////////////////////////////////////////////

  void Algorithm::Stop()
  {
    m_log->Trace(fmt::format("/{}\\", Logger::Header("StopHook")));
    StopHook();
    m_log->Trace(fmt::format("\\{:=^50}/", ""));
  }

  ///////////////////////////////////////////////////////////////////////////////

  template <typename OPTION_TYPE>
  OPTION_TYPE Algorithm::GetOptionScalar(YAMLReader::node_path_t node_path) const
  {
    auto key = YAMLReader::NodePath2String(node_path);
    auto opt = GetCachedOption<OPTION_TYPE>(key);
    node_path.push_front(m_class_name);
    if(!opt.has_value()) {
      opt = m_yaml_config->GetScalar<OPTION_TYPE>(node_path);
    }
    if(!opt.has_value()) {
      throw std::runtime_error(fmt::format("Failed to get scalar option for parameter {:?} for algorithm {:?}", key, m_class_name));
    }
    PrintOptionValue(key, opt.value());
    return opt.value();
  }
  template int Algorithm::GetOptionScalar(YAMLReader::node_path_t node_path) const;
  template double Algorithm::GetOptionScalar(YAMLReader::node_path_t node_path) const;
  template std::string Algorithm::GetOptionScalar(YAMLReader::node_path_t node_path) const;

  ///////////////////////////////////////////////////////////////////////////////

  template <typename OPTION_TYPE>
  std::vector<OPTION_TYPE> Algorithm::GetOptionVector(YAMLReader::node_path_t node_path) const
  {
    auto key = YAMLReader::NodePath2String(node_path);
    auto opt = GetCachedOption<std::vector<OPTION_TYPE>>(key);
    node_path.push_front(m_class_name);
    if(!opt.has_value()) {
      opt = m_yaml_config->GetVector<OPTION_TYPE>(node_path);
    }
    if(!opt.has_value()) {
      throw std::runtime_error(fmt::format("Failed to get vector option for parameter {:?} for algorithm {:?}", key, m_class_name));
    }
    PrintOptionValue(key, opt.value());
    return opt.value();
  }
  template std::vector<int> Algorithm::GetOptionVector(YAMLReader::node_path_t node_path) const;
  template std::vector<double> Algorithm::GetOptionVector(YAMLReader::node_path_t node_path) const;
  template std::vector<std::string> Algorithm::GetOptionVector(YAMLReader::node_path_t node_path) const;

  ///////////////////////////////////////////////////////////////////////////////

  template <typename OPTION_TYPE>
  std::set<OPTION_TYPE> Algorithm::GetOptionSet(YAMLReader::node_path_t node_path) const
  {
    auto val_vec = GetOptionVector<OPTION_TYPE>(node_path);
    std::set<OPTION_TYPE> val_set;
    std::copy(val_vec.begin(), val_vec.end(), std::inserter(val_set, val_set.end()));
    return val_set;
  }
  template std::set<int> Algorithm::GetOptionSet(YAMLReader::node_path_t node_path) const;
  template std::set<double> Algorithm::GetOptionSet(YAMLReader::node_path_t node_path) const;
  template std::set<std::string> Algorithm::GetOptionSet(YAMLReader::node_path_t node_path) const;

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
      try {
        m_yaml_config->AddFile(m_default_config_file, false);
      }
      catch(std::runtime_error const& ex) {
        m_log->Debug("this algorithm has no default configuration YAML file");
      }
      m_yaml_config->AddFile(o_user_config_file);
    }
    else
      m_log->Debug("`YAMLReader` already instantiated for this algorithm; using that");

    // parse the files
    m_yaml_config->LoadFiles();

    // set log level
    try {
      auto log_level = GetOptionScalar<std::string>({"log"});
      m_log->SetLevel(log_level);
      m_yaml_config->SetLogLevel(log_level);
    }
    catch(std::runtime_error const& ex) {
      PrintOptionValue("log", m_log->GetLevelName() + " (default)");
    }
  }

  ///////////////////////////////////////////////////////////////////////////////

  void Algorithm::StartRCDBReader()
  {
    m_rcdb = std::make_unique<RCDBReader>("RCDB|" + GetName(), m_log->GetLevel());
  }

  ///////////////////////////////////////////////////////////////////////////////

  hipo::banklist::size_type Algorithm::GetBankIndex(hipo::banklist& banks, std::string const& bank_name) const
  {
    if(m_rows_only)
      return 0;
    try {
      // check if this bank was created by iguana
      auto created_by_iguana = AlgorithmFactory::GetCreatorAlgorithms(bank_name);
      // get the index
      auto idx = tools::GetBankIndex(
          banks,
          bank_name,
          created_by_iguana ? m_created_bank_variant : 0);
      m_log->Trace("cached index of bank '{}' is {}", bank_name, idx);
      return idx;
    }
    catch(std::runtime_error const& ex) {
      m_log->Error("required input bank '{}' not found; cannot `Start` algorithm '{}'", bank_name, m_class_name);
      auto creators = AlgorithmFactory::GetCreatorAlgorithms(bank_name);
      if(creators)
        m_log->Error(" -> this bank is created by algorithm(s) [{}]; please `Start` ONE of them BEFORE this algorithm", fmt::join(creators.value(), ", "));
      throw std::runtime_error("cannot cache bank index");
    }
  }

  ///////////////////////////////////////////////////////////////////////////////

  hipo::banklist::size_type Algorithm::GetCreatedBankIndex(hipo::banklist& banks) const noexcept(false)
  {
    return GetBankIndex(banks, GetCreatedBankName());
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
        auto creators = AlgorithmFactory::GetCreatorAlgorithms(expected_bank_name);
        if(creators)
          m_log->Error(" -> this bank is created by algorithm(s) [{}]; please `Run` ONE of them BEFORE this algorithm", fmt::join(creators.value(), ", "));
      }
    }
    throw std::runtime_error("GetBank failed");
  }

  ///////////////////////////////////////////////////////////////////////////////

  std::vector<std::string> Algorithm::GetCreatedBankNames() const noexcept(false)
  {
    auto created_banks = AlgorithmFactory::GetCreatedBanks(m_class_name);
    if(created_banks)
      return created_banks.value();
    throw std::runtime_error("failed to get created bank names");
  }

  ///////////////////////////////////////////////////////////////////////////////

  std::string Algorithm::GetCreatedBankName() const noexcept(false)
  {
    auto created_banks = GetCreatedBankNames();
    switch(created_banks.size()) {
    case 0:
      m_log->Error("algorithm {:?} creates no new banks", m_class_name);
      break;
    case 1:
      return created_banks.at(0);
      break;
    default:
      m_log->Error("algorithm {:?} creates more than one bank; they are: [{}]", m_class_name, fmt::join(created_banks, ", "));
      m_log->Error("- if you called `GetCreatedBank` or `GetCreatedBankSchema`, please specify which bank you want");
      m_log->Error("- if you called `GetCreatedBankName`, call `GetCreatedBankNames` instead");
      break;
    }
    throw std::runtime_error("failed to get created bank names");
  }

  ///////////////////////////////////////////////////////////////////////////////

  hipo::bank Algorithm::GetCreatedBank(std::string const& bank_name) const noexcept(false)
  {
    return hipo::bank(GetCreatedBankSchema(bank_name), 0);
  }

  ///////////////////////////////////////////////////////////////////////////////

  hipo::schema Algorithm::GetCreatedBankSchema(std::string const& bank_name) const noexcept(false)
  {
    std::string bank_name_arg = bank_name; // copy, to permit modification

    // if the user did not provide a bank name, get it from the list of banks created by the algorithm;
    // this will fail if the algorithm creates more than one bank, in which case, the user must
    // specify the bank name explicitly
    if(bank_name.empty())
      bank_name_arg = GetCreatedBankName();

    // loop over bank definitions, `BANK_DEFS`, which is generated at build-time using `src/iguana/bankdefs/iguana.json`
    for(auto const& bank_def : BANK_DEFS) {
      if(bank_def.name == bank_name_arg) {
        // make sure the new bank is in REGISTER_IGUANA_ALGORITHM
        if(!AlgorithmFactory::GetCreatorAlgorithms(bank_name_arg)) {
          m_log->Error("algorithm {:?} creates bank {:?}, which is not registered; new banks must be included in `REGISTER_IGUANA_ALGORITHM` arguments", m_class_name, bank_name_arg);
          throw std::runtime_error("CreateBank failed");
        }
        // create the schema format string
        std::vector<std::string> schema_def;
        for(auto const& entry : bank_def.entries)
          schema_def.push_back(entry.name + "/" + entry.type);
        auto format_string = fmt::format("{}", fmt::join(schema_def, ","));
        // create the new bank schema
        hipo::schema bank_schema(bank_name_arg.c_str(), bank_def.group, bank_def.item);
        bank_schema.parse(format_string);
        return bank_schema;
      }
    }

    throw std::runtime_error(fmt::format("bank {:?} not found in 'BankDefs.h'; is this bank defined in src/iguana/bankdefs/iguana.json ?", bank_name_arg));
  }
  ///////////////////////////////////////////////////////////////////////////////

  unsigned int Algorithm::GetCreatedBankVariant() const
  {
    return m_created_bank_variant;
  }

  ///////////////////////////////////////////////////////////////////////////////

  std::unique_ptr<RCDBReader>& Algorithm::GetRCDBReader()
  {
    return m_rcdb;
  }

  ///////////////////////////////////////////////////////////////////////////////

  hipo::schema Algorithm::CreateBank(
      hipo::banklist& banks,
      hipo::banklist::size_type& bank_idx,
      std::string const& bank_name) noexcept(false)
  {
    // check if this bank is already present in `banks`, and set `m_created_bank_variant` accordingly
    for(auto& bank : banks) {
      if(bank.getSchema().getName() == bank_name)
        m_created_bank_variant++;
    }
    // create the schema, and add the new bank to `banks`
    auto bank_schema = GetCreatedBankSchema(bank_name);
    bank_idx         = banks.size();
    banks.emplace_back(bank_schema, 0);
    return bank_schema;
  }

  ///////////////////////////////////////////////////////////////////////////////

  void Algorithm::ShowBanks(hipo::banklist const& banks, std::string_view message, Logger::Level const level) const
  {
    if(m_log->GetLevel() <= level) {
      if(!message.empty())
        m_log->Print(level, message);
      for(auto& bank : banks)
        bank.show();
    }
  }

  ///////////////////////////////////////////////////////////////////////////////

  void Algorithm::ShowBank(hipo::bank const& bank, std::string_view message, Logger::Level const level) const
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

  void Algorithm::ThrowSinceRenamed(std::string const& new_name, std::string const& version) const noexcept(false)
  {
    std::string new_path      = new_name;
    std::string::size_type it = 0;
    while((it = new_path.find("::", it)) != std::string::npos)
      new_path.replace(it, 2, "/");
    m_log->Error("As of Iguana version {}, the algorithm {:?} has been renamed:", version, m_class_name);
    m_log->Error("- the new name is: {:?}", new_name);
    m_log->Error("- the new C++ header is: \"iguana/algorithms/{}/Algorithm.h\"", new_path);
    m_log->Error("- please update your code (and custom configuration YAML, if you have one)");
    m_log->Error("- sorry for the inconvenience!");
    throw std::runtime_error("algorithm has been renamed");
  }

}
