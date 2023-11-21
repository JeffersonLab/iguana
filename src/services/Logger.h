#pragma once

#include <fmt/format.h>
#include <unordered_map>

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
      static const Level defaultLevel = info;

      Logger(std::string name = "log", Level lev = defaultLevel);
      ~Logger() {}

      void SetLevel(Level lev);
      Level GetLevel();

      template <typename... VALUES> void Trace(std::string message, VALUES... vals) { Print(trace, message, vals...); }
      template <typename... VALUES> void Debug(std::string message, VALUES... vals) { Print(debug, message, vals...); }
      template <typename... VALUES> void Info(std::string  message, VALUES... vals) { Print(info,  message, vals...); }
      template <typename... VALUES> void Warn(std::string  message, VALUES... vals) { Print(warn,  message, vals...); }
      template <typename... VALUES> void Error(std::string message, VALUES... vals) { Print(error, message, vals...); }

      template <typename... VALUES>
        void Print(Level lev, std::string message, VALUES... vals) {
          if(lev >= m_level) {
            if(m_level_names.contains(lev)) {
              auto prefix = fmt::format("[{}] [{}] ", m_level_names.at(lev), m_name);
              fmt::print(
                  lev >= warn ? stderr : stdout,
                  fmt::runtime(prefix + message + "\n"),
                  vals...
                  );
            } else {
              Warn("Logger::Print called with unknown log level '{}'; printing as error instead", fmt::underlying(lev));
              Error(message, vals...);
            }
          }
        }

    private:
      std::string m_name;
      Level m_level;
      std::unordered_map<Level,std::string> m_level_names;

  };
}
