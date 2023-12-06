#include "Iguana.h"

namespace iguana {

  Iguana::Iguana() {
    algo_map.insert({clas12_EventBuilderFilter, std::move(std::make_unique<clas12::EventBuilderFilter>())});
  }

}
