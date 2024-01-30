#pragma once

#include <deque>
#include "Object.h"

namespace iguana {

  /// @brief Configuration file manager
  class ConfigFileManager : public Object {

    public:

      /// @param name the name of this configuration file handler
      ConfigFileManager(const std::string name="config");

      /// Get the config files' _fixed_ installation prefix
      /// @return the absolute path to the installed configuration file directory
      static std::string GetConfigPrefix();

      /// Add a directory to the configuration files' search paths.
      /// @param dir the directory, which may be relative or absolute
      void AddDirectory(const std::string dir);

      /// Print the list of directories (search path)
      /// @param level the log level
      void PrintDirectories(const Logger::Level level=Logger::info);

      /// Find a configuration file by name. The following locations are searched, in order:
      /// - directories included by `ConfigFileManager::AddDirectory`, starting from the most recently added directory
      /// - the common installation prefix
      /// @param name the configuration file name (without a directory)
      /// @return the found configuration file (with the directory)
      std::string FindFile(const std::string name);

      /// Return the directory containing a file, by stripping the last
      /// component of a file name, similarly to the `dirname` command.
      /// @param name the file name
      /// @return the parent directory name
      static std::string DirName(const std::string name);

    private:

      /// The sequence of algorithms
      std::deque<std::string> m_file_paths;

  };
}
