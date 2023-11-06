#include "FiducialCuts.h"

namespace iguana::clas12 {

  void FiducialCuts::Start() {
    m_log->Info("start fiducial cuts");
  }

  int FiducialCuts::Run(int a, int b) {
    return a+b;
  }

  void FiducialCuts::Stop() {
    m_log->Info("stop fiducial cuts");
  }

}
