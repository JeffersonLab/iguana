#pragma once

#include <memory>
#include <string>

#include <fmt/format.h>
#include "Logger.h"

namespace iguana {

  /// @brief A named object with a `Logger` instance
  class Object
  {

    public:

      /// These are the available log levels, from lowest to highest:
      /// - `trace`: the most verbose level, used for fine-grained printouts for each event
      /// - `debug`: less verbose printout, expected to be less frequent than `trace`
      /// - `info`: the least verbose printout; this is the default level
      /// - `quiet`: use this level to only allow warnings and errors, silencing all other printouts
      /// - `warn`: an issue that may or may not be critical
      /// - `error`: an issue that is likely critical
      /// - `silent`: use this level to silence **all** printouts (use it at your own risk!)
      enum LogLevel {
        trace,
        debug,
        info,
        quiet,
        warn,
        error,
        silent
      };

      /// @param name the name of this object
      Object(std::string_view name = "");
      ~Object() {}

      /// Get the logger
      /// @return the logger used by this object
      std::unique_ptr<Logger>& Log();

      /// Change the name of this object
      /// @param name the new name
      void SetName(std::string_view name);

      /// Get the name of this object
      std::string GetName() const;

      /// Set the log level to this level. Log messages with a lower level will not be printed.
      /// @see `Logger::Level` for available levels.
      /// @param lev the log level name
      void SetLogLevel(std::string_view lev);

      /// Set the log level to this level. Log messages with a lower level will not be printed.
      /// @see `Logger::Level` for available levels.
      /// @param lev the log level
      void SetLogLevel(Logger::Level const lev);


    protected:

      /// The name of this object
      std::string m_name;

      /// The current log level for this instance
      LogLevel m_level;

      /// `Logger` instance for this object
      std::unique_ptr<Logger> m_log;

      void PrintLogV(FILE* out, std::string_view prefix, fmt::string_view fmt_str, fmt::format_args fmt_args)
      {
        fmt::print(out, "{} {}\n", prefix, fmt::vformat(fmt_str, fmt_args));
      }

      template <typename... ARG_TYPES>
      void PrintLog(FILE* out, std::string_view prefix, fmt::format_string<ARG_TYPES...> fmt_str, ARG_TYPES&&... fmt_args)
      {
        PrintLogV(out, prefix, fmt_str, fmt::make_format_args(fmt_args...));
      }

  };
}
