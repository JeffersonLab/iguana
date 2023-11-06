# pragma once

#include <fmt/core.h>

namespace iguana {

  class Logger {

    public:
      Logger(int lev=0);
      ~Logger() {}

      void SetLevel(int lev);
      void Error(std::string msg);

  };
}
