#include "YAMLReader.h"

namespace iguana {
  void YAMLReader::LoadFiles()
  {
    m_log->Debug("Called YAMLReader::LoadFiles()");
    for(const auto& file : m_files) {
      try {
        m_log->Debug(" - Loading YAML file: {}", file);
        m_configs.push_back({YAML::LoadFile(file), file}); // m_config must be the same ordering as m_files, so `push_back`
      }
      catch(const YAML::Exception& e) {
        m_log->Error(" - YAML Exception: {}", e.what());
      }
      catch(const std::exception& e) {
        m_log->Error(" - Exception: {}", e.what());
      }
    }
  }

  template <typename T>
  T YAMLReader::readValue(const std::string& key, T defaultValue, const YAML::Node& node)
  {
    // m_log->AddTag(fmt::format("readValue({})", key));
    for(const auto& [config, filename] : m_configs) {
      try {
        const YAML::Node& targetNode = node.IsNull() ? config : node;
        // m_log->Trace("Searching in {}", node.IsNull() ? filename : "`node` reference");
        if(targetNode[key]) {
          // m_log->Trace(" - FOUND");
          // m_log->RemoveLastTag();
          return targetNode[key].as<T>();
        }
      }
      catch(const YAML::Exception& e) {
        // Handle YAML parsing errors
        // m_log->Error("{} YAML Exception: {}", e.what());
        break;
      }
      catch(const std::exception& e) {
        // Handle other exceptions (e.g., conversion errors)
        // m_log->Error(" - Exception: {}", e.what());
        break;
      }
    }
    // not found in any config
    // m_log->Trace(" - NOT FOUND, using `defaultValue`");
    // m_log->RemoveLastTag();
    return defaultValue;
  }

  // Explicit instantiation for double
  template double YAMLReader::readValue<double>(const std::string&, const double, const YAML::Node&);
  // Explicit instantiation for int
  template int YAMLReader::readValue<int>(const std::string&, const int, const YAML::Node&);
  // Explicit instantiation for std::string
  template std::string YAMLReader::readValue<std::string>(const std::string&, const std::string, const YAML::Node&);

  template <typename T>
  std::vector<T> YAMLReader::readArray(const std::string& key, const YAML::Node& node, const std::string throw_msg)
  {
    m_log->AddTag(fmt::format("readArray({})", key));
    for(const auto& [config, filename] : m_configs) {
      try {
        const YAML::Node& targetNode = node.IsNull() ? config : node;
        // m_log->Trace("Searching in {}", node.IsNull() ? filename : "`node` reference");
        if(targetNode[key]) {
          // m_log->Trace(" - FOUND");
          std::vector<T> value;
          const YAML::Node& arrayNode = targetNode[key];
          for(const auto& element : arrayNode) {
            value.push_back(element.as<T>());
          }
          m_log->RemoveLastTag();
          return value;
        }
      }
      catch(const YAML::Exception& e) {
        // Handle YAML parsing errors
        m_log->Error(" - YAML Exception: {}", e.what());
        break;
      }
      catch(const std::exception& e) {
        // Handle other exceptions (e.g., conversion errors)
        m_log->Error(" - Exception: {}", e.what());
        break;
      }
    }
    // not found in any config
    // m_log->Trace(" - NOT FOUND, using `defaultValue`");
    if(throw_msg!="") {
      m_log->Error("Cannot find this key or its value");
      m_log->RemoveLastTag();
      throw std::runtime_error(throw_msg);
    }
    m_log->RemoveLastTag();
    return {};
  }

  // Explicit instantiation for double
  template std::vector<double> YAMLReader::readArray<double>(const std::string&, const YAML::Node&, const std::string throw_msg);
  // Explicit instantiation for int
  template std::vector<int> YAMLReader::readArray<int>(const std::string&, const YAML::Node&, const std::string throw_msg);
  // Explicit instantiation for std::string
  template std::vector<std::string> YAMLReader::readArray<std::string>(const std::string&, const YAML::Node&, const std::string throw_msg);

  template <typename T>
  T YAMLReader::findKeyAtRunAndPID(
      const std::string& cutKey,
      const std::string& runKey,
      const std::string& pidKey,
      const std::string& key,
      const int runnb,
      const int pid,
      const T defaultValue)
  {
    m_log->AddTag(fmt::format("findKeyAtRunAndPID({}, {})", cutKey, key));
    for(const auto& [config, filename] : m_configs) {
      m_log->Trace("Searching in {}", filename);
      // Accessing the whole sequence of maps
      const YAML::Node& cutsNode = config[cutKey];
      if(cutsNode.IsSequence()) {
        for(const auto& runNode : cutsNode) {
          std::vector<int> runs = readArray<int>(runKey, runNode, "");
          if(runs.size() == 2 && runs[0] <= runnb && runs[1] >= runnb) {
            if(runNode[pidKey].IsDefined()) {
              const YAML::Node& pidNode = runNode[pidKey];
              m_log->Trace(" - FOUND in pidNode");
              m_log->RemoveLastTag();
              return readValue<T>(std::to_string(pid), defaultValue, pidNode);
            }
            else {
              m_log->Trace(" - FOUND in runNode");
              m_log->RemoveLastTag();
              return readValue<T>(key, defaultValue, runNode);
            }
          }
        }
      }
    }
    // not found in any config
    m_log->Trace(" - NOT FOUND, using `defaultValue`");
    m_log->RemoveLastTag();
    return defaultValue;
  }

  // Explicit instantiation for double
  template double YAMLReader::findKeyAtRunAndPID<double>(const std::string&, const std::string&, const std::string&, const std::string&, int, int, const double);
  // Explicit instantiation for int
  template int YAMLReader::findKeyAtRunAndPID<int>(const std::string&, const std::string&, const std::string&, const std::string&, int, int, const int);
  // Explicit instantiation for std::string
  template std::string YAMLReader::findKeyAtRunAndPID<std::string>(const std::string&, const std::string&, const std::string&, const std::string&, int, int, const std::string);

  template <typename T>
  std::vector<T> YAMLReader::findKeyAtRunAndPIDVector(
      const std::string& cutKey,
      const std::string& runKey,
      const std::string& pidKey,
      const std::string& key,
      int runnb,
      int pid)
  {
    m_log->AddTag(fmt::format("findKeyAtRunAndPIDVector({}, {})", cutKey, key));
    for(const auto& [config, filename] : m_configs) {
      m_log->Trace("Searching in {}", filename);
      // Accessing the whole sequence of maps
      const YAML::Node& cutsNode = config[cutKey];
      if(cutsNode.IsSequence()) {
        for(const auto& runNode : cutsNode) {
          std::vector<int> runs = readArray<int>(runKey, runNode, "");
          if(runs.size() == 2 && runs[0] <= runnb && runs[1] >= runnb) {
            if(runNode[pidKey].IsDefined()) {
              const YAML::Node& pidNode = runNode[pidKey];
              m_log->Trace(" - FOUND in pidNode");
              m_log->RemoveLastTag();
              return readArray<T>(std::to_string(pid), pidNode, fmt::format("could not find key '{}' in pidNode", key));
            }
            else {
              m_log->Trace(" - FOUND in runNode");
              m_log->RemoveLastTag();
              return readArray<T>(key, runNode, fmt::format("could not find key '{}' in runNode", key));
            }
          }
        }
      }
    }
    // not found, search for default
    m_log->Trace(" - NOT FOUND, searching for DEFAULT");
    for(const auto& [config, filename] : m_configs) {
      const YAML::Node& cutsNode = config[cutKey];
      m_log->Trace("Searching in {}", filename);
      if(cutsNode.IsSequence()) {
        for(const auto& subNode : cutsNode) {
          if(subNode["default"].IsDefined()) {
            m_log->Trace(" - FOUND");
            m_log->RemoveLastTag();
            return readArray<T>(key, subNode, fmt::format("could not find key '{}' in defaultNode", key));
          }
        }
      }
    }
    m_log->Error("Cannot find this configuration option, or its default value");
    m_log->RemoveLastTag();
    throw std::runtime_error("Failed to find config value");
  }

  // Explicit instantiation for double
  template std::vector<double> YAMLReader::findKeyAtRunAndPIDVector<double>(const std::string&, const std::string&, const std::string&, const std::string&, int, int);
  // Explicit instantiation for int
  template std::vector<int> YAMLReader::findKeyAtRunAndPIDVector<int>(const std::string&, const std::string&, const std::string&, const std::string&, int, int);
  // Explicit instantiation for std::string
  template std::vector<std::string> YAMLReader::findKeyAtRunAndPIDVector<std::string>(const std::string&, const std::string&, const std::string&, const std::string&, int, int);
}
