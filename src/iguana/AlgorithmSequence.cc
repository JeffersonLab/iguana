#include "AlgorithmSequence.h"

namespace iguana {

  AlgorithmSequence::AlgorithmSequence() {
    algo_map.insert({clas12_EventBuilderFilter, std::move(std::make_unique<clas12::EventBuilderFilter>())});
    algo_map.insert({clas12_LorentzTransformer, std::move(std::make_unique<clas12::LorentzTransformer>())});
  }

}
