#include "FiducialCuts.h"
#include <iostream> // TODO: icwyu

namespace iguana::clas12 {

  void FiducialCuts::Start() {
    std::cout << "start fiducial cuts" << std::endl;
  }

  int FiducialCuts::Run(int a, int b) {
    return a+b;
  }

  void FiducialCuts::Stop() {
    std::cout << "stop fiducial cuts" << std::endl;
  }

}
