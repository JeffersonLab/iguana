#pragma once

#include <variant>
#include <vector>

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
      YAMLReader(std::string_view name = "config")
          : ConfigFileReader(name)
      {}
      ~YAMLReader() {}

      /// Parse the YAML files added by `ConfigFileReader::AddFile`
      void LoadFiles();

      /// Read a scalar value from a `YAML::Node`
      /// @param node the `YAML::Node` to read
      /// @return the scalar
      template <typename SCALAR>
      SCALAR GetScalar(YAML::Node node);

      /// Read a scalar value from a `YAML::Node` path; searches all currently loaded config files.
      /// @param node_path the `YAML::Node` path
      /// @return the scalar
      template <typename SCALAR>
      SCALAR GetScalar(node_path_t node_path);

      /// Read a vector value from a `YAML::Node`
      /// @param node the `YAML::Node` to read
      /// @return the vector
      template <typename SCALAR>
      std::vector<SCALAR> GetVector(YAML::Node node);

      /// Read a vector value from a `YAML::Node` path; searches all currently loaded config files.
      /// @param node_path the `YAML::Node` path
      /// @return the vector
      template <typename SCALAR>
      std::vector<SCALAR> GetVector(node_path_t node_path);

      /// Create a function to search a `YAML::Node` for a sub-`YAML::Node` such that
      /// the scalar `val` is within a range specified by `key`
      /// @param key the key of the sub-`YAML::Node` to use as the range (its value must be a 2-vector)
      /// @param val the scalar value to check
      /// @returns the search function
      template <typename SCALAR>
      node_finder_t InRange(std::string const& key, SCALAR val);

    private:

      /// Search a tree of `YAML::Node`s for a node specified by a `node_path_t`
      /// @param node the root `YAML::Node`
      /// @param node_path the path of `YAML::Node` identifiers
      /// @returns either the found `YAML::Node`, or an empty (null) `YAML::Node` if one is not found
      YAML::Node FindNode(YAML::Node node, node_path_t node_path);

      /// Stack of `YAML::Node`s used to open files, together with their file names
      std::deque<std::pair<YAML::Node, std::string>> m_configs;
  };
}
