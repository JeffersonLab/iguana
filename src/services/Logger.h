#pragma once

#include <fmt/format.h>
#include <fmt/color.h>
#include <unordered_map>
#include <functional>

namespace iguana {

  class Logger {

    public:

      // log levels
      enum Level {
        trace,
        debug,
        info,
        warn,
        error
      };
      static const Level DEFAULT_LEVEL = info;

      Logger(std::string name = "log", Level lev = DEFAULT_LEVEL, bool enable_style = true);
      ~Logger() {}

      void SetLevel(std::string lev);
      void SetLevel(Level lev);

      void EnableStyle() { m_enable_style = true; }
      void DisableStyle() { m_enable_style = false; }

      Level GetLevel();
      static std::string Header(std::string message, int width=50);

      template <typename... VALUES> void Trace(std::string message, VALUES... vals) { Print(trace, message, vals...); }
      template <typename... VALUES> void Debug(std::string message, VALUES... vals) { Print(debug, message, vals...); }
      template <typename... VALUES> void Info(std::string  message, VALUES... vals) { Print(info,  message, vals...); }
      template <typename... VALUES> void Warn(std::string  message, VALUES... vals) { Print(warn,  message, vals...); }
      template <typename... VALUES> void Error(std::string message, VALUES... vals) { Print(error, message, vals...); }

      template <typename... VALUES>
        void Print(Level lev, std::string message, VALUES... vals) {
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
      std::string m_name;
      Level m_level;
      std::unordered_map<Level,std::string> m_level_names;
      bool m_enable_style;

  };
}
