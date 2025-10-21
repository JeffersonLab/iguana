#pragma once

#include "iguana/algorithms/clas12/FiducialFilter/Algorithm.h"

namespace iguana::clas12::rgc {

  /// @algo_brief{Filter the `REC::Particle` bank by applying DC (drift chamber) and ECAL (electromagnetic calorimeter) fiducial cuts}
  /// @algo_type_filter
  ///
  /// Based on RG-A algorithm iguana::clas12::FiducialFilter, but with different config for RG-C
  class FiducialFilter : public iguana::clas12::FiducialFilter
  {

      // use `DEFINE_IGUANA_ALGORITHM_IMPL` instead of `DEFINE_IGUANA_ALGORITHM`, since the direct base class is not `Algorithm`
      DEFINE_IGUANA_ALGORITHM_IMPL(FiducialFilter, clas12::rgc::FiducialFilter, clas12::FiducialFilter)

  };

}
