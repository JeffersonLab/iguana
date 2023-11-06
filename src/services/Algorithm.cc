#include "Algorithm.h"

namespace iguana {

  Algorithm::Algorithm(std::string name) {
    m_log = std::make_shared<Logger>(name);
  }

  void Algorithm::StartLogger(std::string name, Logger::Level lev) {
    m_log->SetLevel(lev);
  }

}
