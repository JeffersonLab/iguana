#pragma once

#include <fmt/color.h>
#include <fmt/format.h>
#include <fmt/ranges.h>

#include <functional>
#include <unordered_map>

namespace iguana {

  /// @brief Simple logger service
  ///
  /// - Each algorithm instance should own a `Logger` instance
  /// - The user may control the log level of each `Logger`, thus the log level of each algorithm
  /// - Errors and warnings print to `stderr`, whereas all other levels print to `stdout`
  class Logger
  {

      friend class Object;

    public:

      /// These are the available log levels, from lowest to highest:
      /// - `trace`: the most verbose level, used for fine-grained printouts for each event
      /// - `debug`: less verbose printout, expected to be less frequent than `trace`
      /// - `info`: the least verbose printout; this is the default level
      /// - `quiet`: use this level to only allow warnings and errors, silencing all other printouts
      /// - `warn`: an issue that may or may not be critical
      /// - `error`: an issue that is likely critical
      /// - `silent`: use this level to silence **all** printouts (use it at your own risk!)
      enum Level {
        trace,
        debug,
        info,
        quiet,
        warn,
        error,
        silent
      };

      /// The default log level
      static Level const DEFAULT_LEVEL = info;

      /// @param name the name of this logger instance, which will be include in all of its printouts
      /// @param lev the log level
      /// @param enable_style if true, certain printouts will be styled with color and emphasis
      Logger(std::string_view name = "log", Level const lev = DEFAULT_LEVEL, bool const enable_style = true);
      ~Logger() {}

      /// Set the log level to this level. Log messages with a lower level will not be printed.
      /// @see `Logger::Level` for available levels.
      /// @param lev the log level name
      void SetLevel(std::string_view lev);

      /// Set the log level to this level. Log messages with a lower level will not be printed.
      /// @see `Logger::Level` for available levels.
      /// @param lev the log level
      void SetLevel(Level const lev);

      /// Get the current log level
      /// @returns the log level
      Level GetLevel();

      /// Enable styled log printouts, with color and emphasis
      void EnableStyle();

      /// Disable styled log printout color and emphasis
      void DisableStyle();

      /// Generate a header for a printout
      /// @param message the header message
      /// @param width the width of the header in number of characters
      /// @returns the header string
      static std::string Header(std::string_view message, int const width = 50);

      /// Printout a log message at the `trace` level @see `Logger::Print` for more details
      template <typename... VALUES>
      void Trace(std::string_view message, const VALUES... vals) const { Print(trace, message, vals...); }
      /// Printout a log message at the `debug` level @see `Logger::Print` for more details
      template <typename... VALUES>
      void Debug(std::string_view message, const VALUES... vals) const { Print(debug, message, vals...); }
      /// Printout a log message at the `info` level @see `Logger::Print` for more details
      template <typename... VALUES>
      void Info(std::string_view message, const VALUES... vals) const { Print(info, message, vals...); }
      /// Printout a log message at the `warn` level @see `Logger::Print` for more details
      template <typename... VALUES>
      void Warn(std::string_view message, const VALUES... vals) const { Print(warn, message, vals...); }
      /// Printout a log message at the `error` level @see `Logger::Print` for more details
      template <typename... VALUES>
      void Error(std::string_view message, const VALUES... vals) const { Print(error, message, vals...); }

      /// Printout a log message at the specified level. The message will only print if `lev` is at least as high as the current level of
      /// this `Logger` instance, as set by `Logger::SetLevel`.
      /// @param lev the log level for this message
      /// @param message the message to print; this may be a format string, as in `fmt::format`
      /// @param vals values for the format string `message`
      template <typename... VALUES>
      void Print(Level const lev, std::string_view message, const VALUES... vals) const
      {
        if(lev >= m_level) {
          if(auto it{m_level_names.find(lev)}; it != m_level_names.end()) {
            std::function<std::string(std::string)> style = [](std::string s) { return fmt::format("[{}]", s); };
            if(m_enable_style) {
              switch(lev) {
              case warn:
                style = [](std::string s) { return fmt::format("[{}]", fmt::styled(s, fmt::emphasis::bold | fmt::fg(fmt::terminal_color::magenta))); };
                break;
              case error:
                style = [](std::string s) { return fmt::format("[{}]", fmt::styled(s, fmt::emphasis::bold | fmt::fg(fmt::terminal_color::red))); };
                break;
              default:
                style = [](std::string s) { return fmt::format("[{}]", fmt::styled(s, fmt::emphasis::bold)); };
              }
            }
            fmt::print(
                lev >= warn ? stderr : stdout,
                fmt::runtime(fmt::format("{} {} {}\n", style(it->second), style(m_name), message)),
                vals...);
          }
          else {
            Warn("Logger::Print called with unknown log level '{}'; printing as error instead", static_cast<int>(lev)); // FIXME: static_cast -> fmt::underlying, but needs new version of fmt
            Error(message, vals...);
          }
        }
      }

    private:

      /// The name of this logger, which is included in all printouts
      std::string m_name;

      /// The current log level for this instance
      Level m_level;

      /// Association of the log level to its name
      std::unordered_map<Level, std::string> m_level_names;

      /// If true, style the printouts
      bool m_enable_style;
  };
}
