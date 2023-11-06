# pragma once

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

      template <typename... VALUES> void Trace(std::string msg, VALUES... vals) { Print(trace, msg, vals...); }
      template <typename... VALUES> void Debug(std::string msg, VALUES... vals) { Print(debug, msg, vals...); }
      template <typename... VALUES> void Info(std::string  msg, VALUES... vals) { Print(info,  msg, vals...); }
      template <typename... VALUES> void Warn(std::string  msg, VALUES... vals) { Print(warn,  msg, vals...); }
      template <typename... VALUES> void Error(std::string msg, VALUES... vals) { Print(error, msg, vals...); }

      template <typename... VALUES>
        void Print(Level lev, std::string msg, VALUES... vals) {
          if(lev >= m_level) {
            auto level_name_it = m_level_names.find(lev);
            if(level_name_it == m_level_names.end()) {
              Warn("Logger::Print called with unknown log level '{}'; printing as error instead", fmt::underlying(lev));
              Error(msg, vals...);
            } else {
              fmt::print(
                  lev >= warn ? stderr : stdout,
                  fmt::format(
                    "[{}] [{}] {}\n",
                    level_name_it->second,
                    m_name,
                    fmt::format(msg, vals...)
                    )
                  );
            }
          }
        }

    private:
      std::string m_name;
      Level m_level;
      std::unordered_map<Level,std::string> m_level_names;

  };
}
