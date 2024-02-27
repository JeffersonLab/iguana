#pragma once

#include "Object.h"
#include <deque>

namespace iguana {

  /// @brief Configuration file manager
  class ConfigFileReader : public Object
  {

    public:

      /// @param name the name of this configuration file handler
      ConfigFileReader(const std::string name = "config");

      /// Get the config files' _fixed_ installation prefix
      /// @return the absolute path to the installed configuration file directory
      static std::string GetConfigInstallationPrefix();

      /// Add a directory to the configuration files' search paths.
      /// @param dir the directory, which may be relative or absolute
      void AddDirectory(const std::string dir);

      /// Add a configuration file to be parsed
      /// @param name the name of the file
      void AddFile(const std::string name);

      /// Print the list of directories (search path)
      /// @param level the log level
      void PrintDirectories(const Logger::Level level = Logger::info);

      /// Find a configuration file by name. You may either give just a file name, or specify the full path and filename.
      /// The following locations are searched, in order:
      /// - current working directory `./`
      /// - directories included by `ConfigFileReader::AddDirectory`, starting from the most recently added directory
      /// - the common installation prefix
      /// @param name the configuration file name (with or without a directory)
      /// @return the found configuration file (with the directory)
      std::string FindFile(const std::string name);

      /// Return the directory containing a file, by stripping the last
      /// component of a file name, similarly to the `dirname` command.
      /// @param name the file name
      /// @return the parent directory name
      static std::string DirName(const std::string name);

      /// Convert a full algorithm name to its corresponding default config file name
      /// @param algo_name the algorithm name
      /// @param ext the file extension
      /// @return the config file name
      static std::string ConvertAlgoNameToConfigName(const std::string algo_name, const std::string ext = "yaml");

    protected:

      /// Stack of directories  to search for a file
      std::deque<std::string> m_directories;

      /// Stack of file names to parse
      std::deque<std::string> m_files;
  };
}
