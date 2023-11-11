#pragma once

#include "Logger.h"

namespace iguana {

  class Algorithm {

    public:
      Algorithm(std::string name);
      virtual void Start() = 0;
      virtual int Run(int a, int b) = 0;
      virtual void Stop() = 0;
      virtual ~Algorithm() {}

    protected:
      std::shared_ptr<Logger> m_log;
  };
}
