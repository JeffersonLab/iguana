#include "Logger.h"

namespace iguana {

  Logger::Logger(std::string name, Level lev, bool enable_style) : m_name(name), m_enable_style(enable_style) {
    m_level_names = {
      { trace, "trace" },
      { debug, "debug" },
      { info,  "info"  },
      { warn,  "warn"  },
      { error, "error" }
    };
    SetLevel(lev);
  }

  void Logger::SetLevel(std::string lev) {
    for(auto& [lev_i, lev_n] : m_level_names) {
      if(lev == lev_n) {
        SetLevel(lev_i);
        return;
      }
    }
    Error("Log level '{}' is not a known log level; the log level will remain at '{}'", lev, m_level_names.at(m_level));
  }

  void Logger::SetLevel(Level lev) {
    m_level = lev;
    Debug("log level set to '{}'", m_level_names.at(m_level));
  }

  Logger::Level Logger::GetLevel() {
    return m_level;
  }

  std::string Logger::Header(std::string message, int width) {
    return fmt::format("{:=^{}}", " " + message + " ", width);
  }

}
