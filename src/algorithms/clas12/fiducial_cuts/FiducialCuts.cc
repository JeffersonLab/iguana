#include "FiducialCuts.h"
#include <iostream> // TODO: icwyu

namespace iguana::clas12 {

  void FiducialCuts::Start() {
    StartLogger("fiducial_cuts", spdlog::level::info);
    m_log->info("start fiducial cuts");
  }

  int FiducialCuts::Run(int a, int b) {
    return a+b;
  }

  void FiducialCuts::Stop() {
    std::cout << "stop fiducial cuts" << std::endl;
  }

}
