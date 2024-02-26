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

      /// Read a scalar value from a `YAML::Node`
      /// @param node the `YAML::Node` to read
      /// @return the scalar
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

      /// Read a scalar value from a `YAML::Node` path; searches all currently loaded config files.
      /// @param node_path the `YAML::Node` path
      /// @return the scalar
      template <typename SCALAR>
      SCALAR GetScalar(node_path_t node_path) {
        for(const auto& [config, filename] : m_configs) {
          auto node = FindNode(config, node_path);
          if(node.IsDefined() && !node.IsNull())
            return GetScalar<SCALAR>(node);
        }
        throw std::runtime_error("Failed `GetScalar`");
      }

      /// Read a vector value from a `YAML::Node`
      /// @param node the `YAML::Node` to read
      /// @return the vector
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

      /// Read a vector value from a `YAML::Node` path; searches all currently loaded config files.
      /// @param node_path the `YAML::Node` path
      /// @return the vector
      template <typename SCALAR>
      std::vector<SCALAR> GetVector(node_path_t node_path) {
        for(const auto& [config, filename] : m_configs) {
          auto node = FindNode(config, node_path);
          if(node.IsDefined() && !node.IsNull())
            return GetVector<SCALAR>(node);
        }
        throw std::runtime_error("Failed `GetVector`");
      }

      /// Create a function to search a `YAML::Node` for a sub-`YAML::Node` such that
      /// the scalar `val` is within a range specified by `key`
      /// @param key the key of the sub-`YAML::Node` to use as the range (its value must be a 2-vector)
      /// @param val the scalar value to check
      /// @returns the search function
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

    private:

      /// Search a tree of `YAML::Node`s for a node specified by a `node_path_t`
      /// @param node the root `YAML::Node`
      /// @param node_path the path of `YAML::Node` identifiers
      /// @returns either the found `YAML::Node`, or an empty (null) `YAML::Node` if one is not found
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

      /// Stack of `YAML::Node`s used to open files, together with their file names
      std::deque<std::pair<YAML::Node, std::string>> m_configs;
  };
}
