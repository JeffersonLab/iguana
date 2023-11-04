# pragma once

#include "Logger.h"

namespace iguana {

  class Algorithm {

    public:
      Algorithm();
      virtual void Start() = 0;
      virtual int Run(int a, int b) = 0;
      virtual void Stop() = 0;
      virtual ~Algorithm() {}

    protected:
      void StartLogger(std::string name, spdlog::level::level_enum lev=spdlog::level::info);
      LoggerType m_log;

    private:
      std::shared_ptr<Logger> m_logger;
  };
}
