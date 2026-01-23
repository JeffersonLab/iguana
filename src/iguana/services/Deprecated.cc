#include "Deprecated.h"

#include <map>
#include <stdexcept>

namespace iguana::deprecated {

  void CheckSetOptionKey(std::string const& algo_class_name, std::string const& key) noexcept(false)
  {
    // clang-format off
    std::map<std::string,std::map<std::string,std::string>> renames = {
      {
        "clas12::rga::FiducialFilterPass2",
        {
          { "calorimeter.strictness",      "calorimeter/strictness" },
          { "forward_tagger.radius",       "forward_tagger/radius" },
          { "forward_tagger.holes_flat",   "forward_tagger/holes_flat" },
          { "cvt.edge_layers",             "cvt/edge_layers" },
          { "cvt.edge_min",                "cvt/edge_min" },
          { "cvt.phi_forbidden_deg",       "cvt/phi_forbidden_deg" },
          { "dc.theta_small_deg",          "dc/theta_small_deg" },
          { "dc.thresholds_out",           "dc/thresholds_out" },
          { "dc.thresholds_in_smallTheta", "dc/thresholds_in_smallTheta" },
          { "dc.thresholds_in_largeTheta", "dc/thresholds_in_largeTheta" },
        }
      },
      {
        "clas12::ZVertexFilter",
        {
          {"electron_vz", "electron/vz"},
        }
      },
      {
        "physics::InclusiveKinematics",
        {
          { "reconstruction",  "method/reconstruction" },
          { "lepton_finder",   "method/lepton_finder" },
          { "beam_particle",   "method/beam_particle" },
          { "beam_direction",  "initial_state/beam_direction" },
          { "target_particle", "initial_state/target_particle" },
        }
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
