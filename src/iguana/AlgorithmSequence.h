#pragma once

#include "services/Algorithm.h"

// TODO: avoid listing the algos
#include "algorithms/clas12/event_builder_filter/EventBuilderFilter.h"

namespace iguana {

  /// @brief User-level class for running a sequence of algorithms
  class AlgorithmSequence {

    public:

      AlgorithmSequence();
      ~AlgorithmSequence() {}

      /// Algorithm enumerator
      // TODO: avoid listing the algos
      // TODO: who should own the algorithm instances: AlgorithmSequence or the user?
      enum algo {
        clas12_EventBuilderFilter
      };

      /// Map of algorithm enumerator to the algorithm
      // TODO: make private
      std::unordered_map<AlgorithmSequence::algo, std::unique_ptr<Algorithm>> algo_map;

  };
}
