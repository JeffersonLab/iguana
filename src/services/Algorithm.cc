#include "Algorithm.h"

namespace iguana {

  Algorithm::Algorithm() {
    m_logger = std::make_shared<Logger>();
  }

  void Algorithm::StartLogger(std::string name, spdlog::level::level_enum lev) {
    m_log = m_logger->Clone(name,lev);
  }

}
