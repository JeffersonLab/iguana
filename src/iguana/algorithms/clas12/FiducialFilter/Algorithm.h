#pragma once

#include "iguana/algorithms/clas12/rga/FiducialFilterPass1/Algorithm.h"

namespace iguana::clas12 {

  /// @algo_brief{RENAMED to iguana::clas12::rga::FiducialFilterPass1}
  /// @deprecated This algorithm has been RENAMED to iguana::clas12::rga::FiducialFilterPass1, as of Iguana version 1.0.0.
  class FiducialFilter : public rga::FiducialFilterPass1
  {
      // the deprecated algorithm should inherit from the renamed algorithm, to avoid downstream build failures
      DEFINE_IGUANA_SUBALGORITHM(FiducialFilter, clas12::FiducialFilter, clas12::rga::FiducialFilterPass1)

    public:

      // make sure base-class specialized `Run` functions (overloads) are not shadowed by any `Run` function overrides here
      using rga::FiducialFilterPass1::Run;

      // override `Start`, `Run` and `Stop`
      // provide the new name and the Iguana version number, for a runtime failure
      DEPRECATE_IGUANA_ALGORITHM(ThrowSinceRenamed("clas12::rga::FiducialFilterPass1", "1.0.0");)
  };

}
