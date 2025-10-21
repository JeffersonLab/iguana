#pragma once

#include "iguana/algorithms/clas12/rga/FiducialFilterPass2/Algorithm.h"

namespace iguana::clas12 {

  /// @algo_brief{RENAMED to iguana::clas12::rga::FiducialFilterPass2}
  /// @deprecated This algorithm has been RENAMED to iguana::clas12::rga::FiducialFilterPass2, as of Iguana version 1.0.0.
  class RGAFiducialFilter : public rga::FiducialFilterPass2
  {
    // the deprecated algorithm should inherit from the renamed algorithm, to avoid downstream build failures
    DEFINE_IGUANA_SUBALGORITHM(RGAFiducialFilter, clas12::RGAFiducialFilter, clas12::rga::FiducialFilterPass2)

    // provide the new name and the Iguana version number, for a runtime failure
    DEPRECATE_IGUANA_ALGORITHM(ThrowSinceRenamed("clas12::rga::FiducialFilterPass2", "1.0.0");)
  };

}
