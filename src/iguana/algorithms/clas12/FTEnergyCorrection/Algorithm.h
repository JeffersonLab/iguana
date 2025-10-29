#pragma once

#include "iguana/algorithms/clas12/rga/FTEnergyCorrection/Algorithm.h"

namespace iguana::clas12 {

  /// @algo_brief{RENAMED to iguana::clas12::rga::FTEnergyCorrection}
  /// @algo_type_transformer
  /// @deprecated This algorithm has been RENAMED to iguana::clas12::rga::FTEnergyCorrection, as of Iguana version 1.0.0.
  class FTEnergyCorrection : public rga::FTEnergyCorrection
  {
      // the deprecated algorithm should inherit from the renamed algorithm, to avoid downstream build failures
      DEFINE_IGUANA_SUBALGORITHM(FTEnergyCorrection, clas12::FTEnergyCorrection, clas12::rga::FTEnergyCorrection)

    public:

      // make sure base-class specialized `Run` functions (overloads) are not shadowed by any `Run` function overrides here
      using rga::FTEnergyCorrection::Run;

      // override `Start`, `Run` and `Stop`
      // provide the new name and the Iguana version number, for a runtime failure
      DEPRECATE_IGUANA_ALGORITHM(ThrowSinceRenamed("clas12::rga::FTEnergyCorrection", "1.0.0");)
  };

}
