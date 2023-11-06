#include "Algorithm.h"

namespace iguana {

  Algorithm::Algorithm() {
    m_log = std::make_shared<Logger>();
  }

  void Algorithm::StartLogger(std::string name, int lev) {
    m_log->SetLevel(lev);
  }

}
