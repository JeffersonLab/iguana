#include "Logger.h"

namespace iguana {

  Logger::Logger(std::string name, Level lev) : m_name(name) {
    m_level_names = {
      { trace, "trace" },
      { debug, "debug" },
      { info,  "info"  },
      { warn,  "warn"  },
      { error, "error" }
    };
    SetLevel(lev);
  }

  void Logger::SetLevel(Level lev) {
    m_level = lev;
    Debug("Logger '{}' set to '{}'", m_name, m_level_names.at(m_level));
  }

}
