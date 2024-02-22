#pragma once

#include <string>
#include <vector>

#include <yaml-cpp/yaml.h>

#include "ConfigFileReader.h"

namespace iguana {

  /// @brief A YAMLReader based on yaml-cpp
  class YAMLReader : public ConfigFileReader
  {

    public:
      /// @param name of this reader (for `Logger`)
      YAMLReader(const std::string name = "config")
          : ConfigFileReader(name)
      {}
      ~YAMLReader() {}

      /// Parse the YAML files added by `ConfigFileReader::AddFile`
      void LoadFiles();

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
      /// @param defaultValue the function will default to this if the key does not exist.
      /// @param node the node to read, defaults to the config file opened when the class is instantiated
      /// but this also allows to read from a node within the config file.
      /// @returns the array at the key in the YAML file returned as a `std::vector`.
      template <typename T>
      std::vector<T> readArray(const std::string& key, const std::vector<T>& defaultValue, const YAML::Node& node = YAML::Node());

      /// Iterate over the opened YAML file until the run number corresponds to a node.
      /// The correspondance is found by checking that the run number is greater than
      /// or lesser than the first and second element of the node at key `runkey`.
      /// If the node has a `pidkey` entry, the value is read for the specified pid.
      /// Otherwise the value is read for the run range.
      /// This function can return in any C++ type used by Iguana.
      /// @param cutKey the key that relates to the array of cut values.
      /// @param runkey the key related to the run number range.
      /// @param pidkey the key related to pids.
      /// @param key the value's key in the YAML file for a given run number.
      /// @param runnb the run number used to find correct key.
      /// @param pid the pid to look for.
      /// @param defaultValue the function will default to this if the key does not exist.
      /// @returns the value at the key in the YAML file returned.
      template <typename T>
      T findKeyAtRunAndPID(
          const std::string& cutKey,
          const std::string& runkey,
          const std::string& pidkey,
          const std::string& key,
          const int runnb,
          const int pid,
          const T defaultValue);

      /// Iterate over the opened YAML file until the run number corresponds to a node.
      /// The correspondance is found by checking that the run number is greater than
      /// or lesser than the first and second element of the node at key `runkey`.
      /// If the node has a `pidkey` entry, the array is read for the specified pid.
      /// Otherwise the array is read for the run range.
      /// This function can return in any C++ type used by Iguana.
      /// @param cutKey the key that relates to the array of cut values.
      /// @param runkey the key related to the run number range.
      /// @param pidkey the key related to pids.
      /// @param key the array's key in the YAML file for a given run number.
      /// @param runnb the run number used to find correct key.
      /// @param pid the pid to look for.
      /// @param defaultValue the function will default to this if the key does not exist.
      /// @returns the array at the key in the YAML file returned as a std::vector.
      template <typename T>
      std::vector<T> findKeyAtRunAndPIDVector(
          const std::string& cutKey,
          const std::string& runkey,
          const std::string& pidkey,
          const std::string& key,
          int runnb,
          int pid,
          const std::vector<T>& defaultValue);

    protected:

      /// Stack of nodes used to open files
      std::deque<YAML::Node> m_configs;
  };
}
