/// @file
/// @brief Filter: fiducial volume cuts

#pragma once

#include "services/Algorithm.h"

namespace iguana::clas12 {

  class FiducialCuts : public Algorithm {

    public:
      FiducialCuts() : Algorithm("fiducial_cuts") {}
      ~FiducialCuts() {}

      void Start() override;
      int Run(int a, int b) override;
      void Stop() override;
  };
}
