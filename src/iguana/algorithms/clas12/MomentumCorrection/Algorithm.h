#pragma once

#include "iguana/algorithms/clas12/rga/MomentumCorrection/Algorithm.h"

namespace iguana::clas12 {

  /// @algo_brief{RENAMED to iguana::clas12::rga::MomentumCorrection}
  /// @deprecated This algorithm has been RENAMED to iguana::clas12::rga::MomentumCorrection, as of Iguana version 1.0.0.
  class MomentumCorrection : public rga::MomentumCorrection
  {
      // the deprecated algorithm should inherit from the renamed algorithm, to avoid downstream build failures
      DEFINE_IGUANA_SUBALGORITHM(MomentumCorrection, clas12::MomentumCorrection, clas12::rga::MomentumCorrection)

    public:

      // make sure base-class specialized `Run` functions (overloads) are not shadowed by any `Run` function overrides here
      using rga::MomentumCorrection::Run;

      // override `Start`, `Run` and `Stop`
      // provide the new name and the Iguana version number, for a runtime failure
      DEPRECATE_IGUANA_ALGORITHM(ThrowSinceRenamed("clas12::rga::MomentumCorrection", "1.0.0");)
  };

}
