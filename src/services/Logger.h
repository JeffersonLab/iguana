/// @file
/// @brief Logging service, for algorithm printouts

#pragma once

#include <fmt/format.h>
#include <fmt/color.h>
#include <unordered_map>
#include <functional>

namespace iguana {

  class Logger {

    public:

      /// Log levels
      /// - these are the available log levels, from lowest
      ///   to highest:
      ///   - `trace`
      ///   - `debug`
      ///   - `info`
      ///   - `warn`
      ///   - `error`
      /// - all log levels that are higher than the specified
      ///   log level (by `SetLevel(...)`) will be printed
      enum Level {
        trace,
        debug,
        info,
        warn,
        error
      };
      static const Level DEFAULT_LEVEL = info;

      Logger(const std::string name = "log", const Level lev = DEFAULT_LEVEL, const bool enable_style = true);
      ~Logger() {}

      void SetLevel(const std::string lev);
      void SetLevel(const Level lev);

      Level GetLevel();

      void EnableStyle();
      void DisableStyle();

      static std::string Header(const std::string message, const int width=50);

      template <typename... VALUES> void Trace(const std::string message, const VALUES... vals) const { Print(trace, message, vals...); }
      template <typename... VALUES> void Debug(const std::string message, const VALUES... vals) const { Print(debug, message, vals...); }
      template <typename... VALUES> void Info(const std::string  message, const VALUES... vals) const { Print(info,  message, vals...); }
      template <typename... VALUES> void Warn(const std::string  message, const VALUES... vals) const { Print(warn,  message, vals...); }
      template <typename... VALUES> void Error(const std::string message, const VALUES... vals) const { Print(error, message, vals...); }

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
      const std::string m_name;
      Level m_level;
      std::unordered_map<Level,std::string> m_level_names;
      bool m_enable_style;

  };
}
