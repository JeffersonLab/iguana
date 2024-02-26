#pragma once

#include <string>
#include <vector>
#include <variant>

#include <yaml-cpp/yaml.h>

#include "ConfigFileReader.h"

namespace iguana {

  /// @brief A YAMLReader based on yaml-cpp
  class YAMLReader : public ConfigFileReader
  {

    public:

      /// A function `f : Node A -> Node B` which searches `YAML::Node A` for a specific `YAML::Node B`, returning it
      using node_finder_t = std::function<YAML::Node(const YAML::Node)>;

      /// Variant for identifying a `YAML::Node`:
      /// - `std::string`: the key name of the node
      /// - `node_finder_t`: how to find the node
      using node_id_t = std::variant<std::string, node_finder_t>;

      /// Representation of a path of `YAML::Node`s in a `YAML::Node` tree, _e.g._, in a YAML file.
      using node_path_t = std::deque<node_id_t>;

      /// @param name of this reader (for `Logger`)
      YAMLReader(const std::string name = "config")
          : ConfigFileReader(name)
      {}
      ~YAMLReader() {}

      /// Parse the YAML files added by `ConfigFileReader::AddFile`
      void LoadFiles();





      YAML::Node FindNode(YAML::Node node, node_path_t node_path) {

        // if `node_path` is empty, we are likely at the end of the node path; end recursion and return `node`
        if(node_path.empty()) {
          m_log->Trace("... found");
          return node;
        }

        // find the next node using the first `node_id_t` in `node_path`
        auto node_id_visitor = [&node, &m_log = this->m_log](auto&& arg) -> YAML::Node {
          using arg_t = std::decay_t<decltype(arg)>;
          // find a node by key name
          if constexpr(std::is_same_v<arg_t,std::string>) {
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



      template <typename SCALAR>
      SCALAR GetScalar(YAML::Node node) {
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

      template <typename SCALAR>
      SCALAR GetScalar(node_path_t node_path) {
        for(const auto& [config, filename] : m_configs) {
          auto node = FindNode(config, node_path);
          if(node.IsDefined() && !node.IsNull())
            return GetScalar<SCALAR>(node);
        }
        throw std::runtime_error("Failed `GetScalar`");
      }

      template <typename SCALAR>
      std::vector<SCALAR> GetVector(YAML::Node node) {
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

      template <typename SCALAR>
      std::vector<SCALAR> GetVector(node_path_t node_path) {
        for(const auto& [config, filename] : m_configs) {
          auto node = FindNode(config, node_path);
          if(node.IsDefined() && !node.IsNull())
            return GetVector<SCALAR>(node);
        }
        throw std::runtime_error("Failed `GetVector`");
      }




      template <typename SCALAR>
      node_finder_t InRange(std::string key, SCALAR val) {
        return [this, &key, &val](YAML::Node node) -> YAML::Node {
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








      /// Read a value from the opened YAML file which is at a given and key.
      /// This function can return in any C++ type used by Iguana.
      /// @param key the value's key in the YAML file.
      /// @param defaultValue the function will default to this if the key does not exist.
      /// @param node the node to read, defaults to the config file opened when the class is instantiated
      /// but this also allows to read from a node within the config file.
      /// @returns the value at the key in the YAML file.
      template <typename T>
      T readValue(const std::string& key, T defaultValue, const YAML::Node& node = YAML::Node());

      /// Read an array from the opened YAML file which is at a given key.
      /// This function can return in any C++ type used by Iguana.
      /// @param key the array's key in the YAML file.
      /// @param node the node to read, defaults to the config file opened when the class is instantiated
      /// but this also allows to read from a node within the config file.
      /// @param throw_msg if non-empty, will throw a runtime exception with this message, if the key's value cannot be found
      /// @returns the array at the key in the YAML file returned as a `std::vector`.
      template <typename T>
      std::vector<T> readArray(const std::string& key, const YAML::Node& node = YAML::Node(), const std::string throw_msg="");

      template <typename T>
      std::vector<T> findArray(const std::string& key)
      {
        return readArray<T>(key, {}, fmt::format("cannot find array for key '{}' in any config file", key));
      }

      /// Iterate over the opened YAML file until the run number corresponds to a node.
      /// The correspondance is found by checking that the run number is greater than
      /// or lesser than the first and second element of the node at key `runKey`.
      /// If the node has a `pidKey` entry, the value is read for the specified pid.
      /// Otherwise the value is read for the run range.
      /// This function can return in any C++ type used by Iguana.
      /// @param cutKey the key that relates to the array of cut values.
      /// @param runKey the key related to the run number range.
      /// @param pidKey the key related to pids.
      /// @param key the value's key in the YAML file for a given run number.
      /// @param runnb the run number used to find correct key.
      /// @param pid the pid to look for.
      /// @param defaultValue the function will default to this if the key does not exist.
      /// @returns the value at the key in the YAML file returned.
      template <typename T>
      T findKeyAtRunAndPID(
          const std::string& cutKey,
          const std::string& runKey,
          const std::string& pidKey,
          const std::string& key,
          const int runnb,
          const int pid,
          const T defaultValue);

      /// Iterate over the opened YAML file until the run number corresponds to a node.
      /// The correspondance is found by checking that the run number is greater than
      /// or lesser than the first and second element of the node at key `runKey`.
      /// If the node has a `pidKey` entry, the array is read for the specified pid.
      /// Otherwise the array is read for the run range.
      /// This function can return in any C++ type used by Iguana.
      /// @param cutKey the key that relates to the array of cut values.
      /// @param runKey the key related to the run number range.
      /// @param pidKey the key related to pids.
      /// @param key the array's key in the YAML file for a given run number.
      /// @param runnb the run number used to find correct key.
      /// @param pid the pid to look for.
      /// @returns the array at the key in the YAML file returned as a std::vector.
      template <typename T>
      std::vector<T> findKeyAtRunAndPIDVector(
          const std::string& cutKey,
          const std::string& runKey,
          const std::string& pidKey,
          const std::string& key,
          int runnb,
          int pid);

    protected:

      /// Stack of `YAML::Node`s used to open files, together with their file names
      std::deque<std::pair<YAML::Node, std::string>> m_configs;
  };
}
