#include "Logger.h"

namespace iguana {

  Logger::Logger(spdlog::level::level_enum lev) {
    m_log = spdlog::default_logger()->clone("iguana");
    SetLevel(lev);
  }

  void Logger::SetLevel(spdlog::level::level_enum lev) {
    m_log->set_level(lev);
  }

  LoggerType Logger::Clone(std::string name, spdlog::level::level_enum lev) {
    auto log = m_log->clone(name);
    log->set_level(lev);
    return log;
  }

}
