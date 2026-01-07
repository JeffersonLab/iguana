#pragma once

#include "Object.h"
#include <deque>

namespace iguana {

  /// @brief Configuration file manager
  class ConfigFileReader : public Object
  {

    public:

      /// @param name the name of this configuration file handler
      /// @param set_default_dirs if true, add the default configuration directories
      ConfigFileReader(std::string_view name = "config", bool set_default_dirs = true);

      /// Get the config files' _fixed_ installation prefix
      /// @warning if the Iguana installation is _relocated_, this directory will **not** be correct,
      /// since it is compiled in the shared library; as a fallback, you may use the environment variable
      /// `$IGUANA_CONFIG_PATH`.
      /// @return the absolute path to the installed configuration file directory
      static std::string GetConfigInstallationPrefix();

      /// Add a directory to the configuration files' search paths.
      /// @param dir the directory, which may be relative or absolute
      void AddDirectory(std::string const& dir);

      /// Add a configuration file to be parsed
      /// @param name the name of the file
      void AddFile(std::string const& name);

      /// Print the list of directories (search path)
      /// @param level the log level
      void PrintDirectories(Logger::Level const level = Logger::info);

      /// Find a configuration file by name. You may either give just a file name, or specify the full path and filename.
      /// The following locations are searched, in order:
      /// - current working directory `./`
      /// - directories included by `ConfigFileReader::AddDirectory`, starting from the most recently added directory
      /// - the common installation prefix
      /// @param name the configuration file name (with or without a directory)
      /// @return the found configuration file (with the directory)
      std::string FindFile(std::string name);

      /// Convert a full algorithm name to its corresponding default config file subdirectory
      /// @param algo_name the algorithm name
      /// @return the config file subdirectory
      static std::string ConvertAlgoNameToConfigDir(std::string_view algo_name);

      /// Convert a full algorithm name to its corresponding default config file name
      /// @param algo_name the algorithm name
      /// @param ext the file extension
      /// @return the config file name
      static std::string ConvertAlgoNameToConfigName(std::string_view algo_name, std::string_view ext = "yaml");

    protected:

      /// Stack of directories  to search for a file
      std::deque<std::string> m_directories;

      /// Stack of file names to parse
      std::deque<std::string> m_files;
  };
}
