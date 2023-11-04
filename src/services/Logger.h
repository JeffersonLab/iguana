# pragma once

#include <spdlog/spdlog.h>

namespace iguana {

  using LoggerType = std::shared_ptr<spdlog::logger>;

  class Logger {

    public:
      Logger(spdlog::level::level_enum lev=spdlog::level::info);
      ~Logger() {}

      void SetLevel(spdlog::level::level_enum lev);
      LoggerType GetLog() { return m_log; }
      LoggerType Clone(std::string name, spdlog::level::level_enum lev);

    private:
      LoggerType m_log;

  };
}
