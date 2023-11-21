#include "Iguana.h"

namespace iguana {

  Iguana::Iguana() {
    algo_map.insert({clas12_EventBuilderFilter, std::make_shared<clas12::EventBuilderFilter>()});
  }

}
