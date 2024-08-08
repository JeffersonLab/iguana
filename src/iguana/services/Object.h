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

      /// `Logger` instance for this object
      std::unique_ptr<Logger> m_log;

      void PrintLogV(std::string_view name, fmt::string_view fmt_str, fmt::format_args fmt_args)
      {
        fmt::print("<{}> {}\n", name, fmt::vformat(fmt_str, fmt_args));
      }

      template <typename... ARG_TYPES>
      void PrintLog(std::string_view name, fmt::format_string<ARG_TYPES...> fmt_str, ARG_TYPES&&... fmt_args)
      {
        PrintLogV(name, fmt_str, fmt::make_format_args(fmt_args...));
      }

  };
}

#define NEWLOG(fmt_str, ...) \
  PrintLog( \
      fmt::format("[{}]", fmt::styled(m_name, fmt::emphasis::bold | fmt::fg(fmt::terminal_color::magenta))), \
      fmt_str, \
      __VA_ARGS__);
