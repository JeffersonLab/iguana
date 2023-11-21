#pragma once

#include "services/Algorithm.h"
#include <unordered_map>

// TODO: avoid listing the algos
#include "algorithms/clas12/event_builder_filter/EventBuilderFilter.h"

namespace iguana {

  class Iguana {

    public:
      Iguana();
      ~Iguana() {}

      // TODO: avoid listing the algos
      // TODO: who should own the algorithm instances: Iguana or the user?
      enum algo {
        clas12_EventBuilderFilter
      };

      // TODO: make private
      std::unordered_map<Iguana::algo, std::shared_ptr<Algorithm>> algo_map;

  };
}
