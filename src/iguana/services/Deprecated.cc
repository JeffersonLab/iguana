#include "Deprecated.h"

#include <map>
#include <stdexcept>

namespace iguana::deprecated {

  void CheckSetOptionKey(std::string const& algo_class_name, std::string const& key) noexcept(false)
  {
    // clang-format off
    std::map<std::string,std::map<std::string,std::string>> renames = {
      { "physics::InclusiveKinematics", {
                                          { "reconstruction",  "method/reconstruction" },
                                          { "lepton_finder",   "method/lepton_finder" },
                                          { "beam_particle",   "method/beam_particle" },
                                          { "beam_direction",  "initial_state/beam_direction" },
                                          { "target_particle", "initial_state/target_particle" },
                                        },
      },
    };
    // clang-format on
    if(auto algo_it{renames.find(algo_class_name)}; algo_it != renames.end()) {
      if(auto rename_it{algo_it->second.find(key)}; rename_it != algo_it->second.end()) {
        throw std::runtime_error("Called 'SetOption' with deprecated key '" + key + "'; it has been renamed to '" + rename_it->second + "'");
      }
    }
  }

}
