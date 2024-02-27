#include "YAMLReader.h"

namespace iguana {

  void YAMLReader::LoadFiles()
  {
    m_log->Debug("YAMLReader::LoadFiles():");
    for(const auto& file : m_files) {
      try {
        m_log->Debug(" - load: {}", file);
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

  ///////////////////////////////////////////////////////////////////////////////

  template <typename SCALAR>
  SCALAR YAMLReader::GetScalar(YAML::Node node)
  {
    if(node.IsDefined() && !node.IsNull()) {
      try {
        return node.as<SCALAR>();
      }
      catch(const YAML::Exception& e) {
        m_log->Error("YAML Parsing Exception: {}", e.what());
      }
      catch(const std::exception& e) {
        m_log->Error("YAML Misc. Exception: {}", e.what());
      }
    }
    throw std::runtime_error("Failed `GetScalar`");
  }
  template int YAMLReader::GetScalar(YAML::Node node);
  template double YAMLReader::GetScalar(YAML::Node node);
  template std::string YAMLReader::GetScalar(YAML::Node node);

  ///////////////////////////////////////////////////////////////////////////////

  template <typename SCALAR>
  SCALAR YAMLReader::GetScalar(node_path_t node_path)
  {
    for(const auto& [config, filename] : m_configs) {
      auto node = FindNode(config, node_path);
      if(node.IsDefined() && !node.IsNull())
        return GetScalar<SCALAR>(node);
    }
    throw std::runtime_error("Failed `GetScalar`");
  }
  template int YAMLReader::GetScalar(node_path_t node_path);
  template double YAMLReader::GetScalar(node_path_t node_path);
  template std::string YAMLReader::GetScalar(node_path_t node_path);

  ///////////////////////////////////////////////////////////////////////////////

  template <typename SCALAR>
  std::vector<SCALAR> YAMLReader::GetVector(YAML::Node node)
  {
    if(node.IsDefined() && !node.IsNull() && node.IsSequence()) {
      try {
        std::vector<SCALAR> result;
        for(const auto& element : node)
          result.push_back(element.as<SCALAR>());
        return result;
      }
      catch(const YAML::Exception& e) {
        m_log->Error("YAML Parsing Exception: {}", e.what());
      }
      catch(const std::exception& e) {
        m_log->Error("YAML Misc. Exception: {}", e.what());
      }
    }
    throw std::runtime_error("Failed `GetVector`");
  }
  template std::vector<int> YAMLReader::GetVector(YAML::Node node);
  template std::vector<double> YAMLReader::GetVector(YAML::Node node);
  template std::vector<std::string> YAMLReader::GetVector(YAML::Node node);

  ///////////////////////////////////////////////////////////////////////////////

  template <typename SCALAR>
  std::vector<SCALAR> YAMLReader::GetVector(node_path_t node_path)
  {
    for(const auto& [config, filename] : m_configs) {
      auto node = FindNode(config, node_path);
      if(node.IsDefined() && !node.IsNull())
        return GetVector<SCALAR>(node);
    }
    throw std::runtime_error("Failed `GetVector`");
  }
  template std::vector<int> YAMLReader::GetVector(node_path_t node_path);
  template std::vector<double> YAMLReader::GetVector(node_path_t node_path);
  template std::vector<std::string> YAMLReader::GetVector(node_path_t node_path);

  ///////////////////////////////////////////////////////////////////////////////

  template <typename SCALAR>
  YAMLReader::node_finder_t YAMLReader::InRange(std::string key, SCALAR val)
  {
    return [this, key, val](YAML::Node node) -> YAML::Node
    {
      if(!node.IsSequence()) {
        m_log->Error("YAML node path expected a sequence at current node");
        throw std::runtime_error("Failed `InRange`");
      }
      // search each sub-node for one with `val` with in the range at `key`
      for(const auto& sub_node : node) {
        auto bounds_node = sub_node[key];
        if(bounds_node.IsDefined()) {
          auto bounds = GetVector<SCALAR>(bounds_node);
          if(bounds.size() == 2 && bounds[0] <= val && bounds[1] >= val)
            return sub_node;
        }
      }
      // fallback to the default node
      for(const auto& sub_node : node) {
        if(sub_node["default"].IsDefined())
          return sub_node;
      }
      // if no default found, return empty
      m_log->Error("No default node for `InRange('{}',{})`", key, val);
      throw std::runtime_error("Failed `InRange`");
    };
  }
  template YAMLReader::node_finder_t YAMLReader::InRange(std::string key, int val);
  template YAMLReader::node_finder_t YAMLReader::InRange(std::string key, double val);

  ///////////////////////////////////////////////////////////////////////////////

  YAML::Node YAMLReader::FindNode(YAML::Node node, node_path_t node_path)
  {

    // if `node_path` is empty, we are likely at the end of the node path; end recursion and return `node`
    if(node_path.empty()) {
      m_log->Trace("... found");
      return node;
    }

    // find the next node using the first `node_id_t` in `node_path`
    auto node_id_visitor = [&node, &m_log = this->m_log](auto&& arg) -> YAML::Node
    {
      using arg_t = std::decay_t<decltype(arg)>;
      // find a node by key name
      if constexpr(std::is_same_v<arg_t, std::string>) {
        m_log->Trace("... by key '{}'", arg);
        return node[arg];
      }
      // find a node using a `node_finder_t`
      else {
        m_log->Trace("... by node finder function");
        return arg(node);
      }
    };
    auto result = std::visit(node_id_visitor, node_path.front());

    // if the resulting node is not defined, return an empty node; callers must check the result
    if(!result.IsDefined())
      return {};

    // recurse to the next element of `node_path`
    node_path.pop_front();
    return FindNode(result, node_path);
  }

}
