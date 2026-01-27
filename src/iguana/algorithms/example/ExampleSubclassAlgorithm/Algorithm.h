#pragma once

#include "iguana/algorithms/clas12/rga/FiducialFilterPass1/Algorithm.h"

namespace iguana::example {

  /// @algo_brief{example demonstrating inheritance from another algorithm}
  /// @algo_type_filter
  class ExampleSubclassAlgorithm : public clas12::rga::FiducialFilterPass1
  {
    // use `DEFINE_IGUANA_SUBALGORITHM` rather than the usual `DEFINE_IGUANA_ALGORITHM`;
    // include the base-class algorithm as an argument
    DEFINE_IGUANA_SUBALGORITHM(ExampleSubclassAlgorithm, example::ExampleSubclassAlgorithm, clas12::rga::FiducialFilterPass1)

    private: // hooks
      // if you don't declare a hook, the base-class algorithm's implementation will be used;
      // in this example, we use all the base-class hooks

  };

}
