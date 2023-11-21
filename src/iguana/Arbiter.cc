#include "Arbiter.h"

namespace iguana {

  Arbiter::Arbiter() {
    algo_map.insert({clas12_EventBuilderFilter, std::make_shared<clas12::EventBuilderFilter>()});
  }

}
