#include "Logger.h"

namespace iguana {

  Logger::Logger(int lev) {
    SetLevel(lev);
  }

  void Logger::SetLevel(int lev) {
    // TODO
  }

  void Logger::Error(std::string msg) {
    fmt::print(msg);
  }
}
