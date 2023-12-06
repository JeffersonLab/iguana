/// @file
/// @brief Logging service, for algorithm printouts

#pragma once

#include <fmt/format.h>
#include <fmt/color.h>
#include <unordered_map>
#include <functional>

namespace iguana {

  /// This is a simple logger service.
  /// - Each algorithm instance should own a `Logger` instance.
  /// - The user may control the log level of each `Logger`, thus the log level of each algorithm
  /// - Errors and warnings print to `stderr`, whereas all other levels print to `stdout`
  class Logger {

    public:

      /// These are the available log levels, from lowest to highest. See the following definition
      /// for the list of them:
      enum Level {
        trace,
        debug,
        info,
        warn,
        error
      };

      /// The default log level
      static const Level DEFAULT_LEVEL = info;

      /// Create a logger instance
      /// @param name the name of this logger instance, which will be include in all of its printouts
      /// @param lev the log level
      /// @param enable_style if true, certain printouts will be styled with color and emphasis
      Logger(const std::string name = "log", const Level lev = DEFAULT_LEVEL, const bool enable_style = true);

      /// Destructor
      ~Logger() {}

      /// Set the log level to this level
      /// @param lev the log level name
      void SetLevel(const std::string lev);

      /// Set the log level to this level
      /// @param lev the log level
      void SetLevel(const Level lev);

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
      static std::string Header(const std::string message, const int width=50);

      /// Printout a log message at the `trace` level; see `Logger::Print` for more details
      template <typename... VALUES> void Trace(const std::string message, const VALUES... vals) const { Print(trace, message, vals...); }
      /// Printout a log message at the `debug` level; see `Logger::Print` for more details
      template <typename... VALUES> void Debug(const std::string message, const VALUES... vals) const { Print(debug, message, vals...); }
      /// Printout a log message at the `info` level; see `Logger::Print` for more details
      template <typename... VALUES> void Info(const std::string  message, const VALUES... vals) const { Print(info,  message, vals...); }
      /// Printout a log message at the `warn` level; see `Logger::Print` for more details
      template <typename... VALUES> void Warn(const std::string  message, const VALUES... vals) const { Print(warn,  message, vals...); }
      /// Printout a log message at the `error` level; see `Logger::Print` for more details
      template <typename... VALUES> void Error(const std::string message, const VALUES... vals) const { Print(error, message, vals...); }

      /// Printout a log message at the specified level. The message will only print if `lev` is at least as high as the current level of
      /// this `Logger` instance, as set by `Logger::SetLevel`.
      /// @param lev the log level for this message
      /// @param message the message to print; this may be a format string, as in `fmt::format`
      /// @param vals values for the format string `message`
      template <typename... VALUES>
        void Print(const Level lev, const std::string message, const VALUES... vals) const {
          if(lev >= m_level) {
            if(auto it{m_level_names.find(lev)}; it != m_level_names.end()) {
              std::string prefix;
              std::function<std::string(std::string)> style = [] (std::string s) { return fmt::format("[{}]", s); };
              if(m_enable_style) {
                switch(lev) {
                  case warn:  style = [] (std::string s) { return fmt::format("[{}]", fmt::styled(s, fmt::emphasis::bold | fmt::fg(fmt::terminal_color::magenta))); }; break;
                  case error: style = [] (std::string s) { return fmt::format("[{}]", fmt::styled(s, fmt::emphasis::bold | fmt::fg(fmt::terminal_color::red)));     }; break;
                  default:    style = [] (std::string s) { return fmt::format("[{}]", fmt::styled(s, fmt::emphasis::bold)); };
                }
              }
              prefix = fmt::format("{} {} ", style(it->second), style(m_name));
              fmt::print(
                  lev >= warn ? stderr : stdout,
                  fmt::runtime(prefix + message + "\n"),
                  vals...
                  );
            } else {
              Warn("Logger::Print called with unknown log level '{}'; printing as error instead", static_cast<int>(lev)); // FIXME: static_cast -> fmt::underlying, but needs new version of fmt
              Error(message, vals...);
            }
          }
        }

    private:

      /// the name of this logger, which is included in all printouts
      const std::string m_name;

      /// the current log level for this instance
      Level m_level;

      /// association of the log level to its name
      std::unordered_map<Level,std::string> m_level_names;

      /// if true, style the printouts
      bool m_enable_style;

  };
}
