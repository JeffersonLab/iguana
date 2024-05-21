/// @file
/// @brief Type definitions for common objects used in algorithms
#pragma once

#include <string>
#include <tuple>
#include <unordered_map>

namespace iguana {

  /// Vector element type
  using vector_element_t = double;
  /// 2-vector container type
  using vector2_t = std::tuple<vector_element_t, vector_element_t>;
  /// 3-vector container type
  using vector3_t = std::tuple<vector_element_t, vector_element_t, vector_element_t>;
  /// 4-vector container type
  using vector4_t = std::tuple<vector_element_t, vector_element_t, vector_element_t, vector_element_t>;

  /// Struct to store trajectory particle data
  struct traj_row_data {
    double x1 = -999;
    double x2 = -999;
    double x3 = -999;
    double y1 = -999;
    double y2 = -999;
    double y3 = -999;
    double z1 = -999;
    double z2 = -999;
    double z3 = -999;
    int sector= 0;
  };    
    
  /// Light-weight namespace for particle constants
  namespace particle {
    // clang-format off

    /// PDG codes
    enum PDG {
      electron = 11,
      pi_plus  = 211,
      pi_minus = -211,
      proton   = 2212
    };

    /// Particle names
    const std::unordered_map<PDG, std::string> name{
      { electron, "electron" },
      { pi_plus,  "pi_plus"  },
      { pi_minus, "pi_minus" },
      { proton,   "proton"   }
    };

    /// Particle titles
    const std::unordered_map<PDG, std::string> title{
      { electron, "e^{-}"   },
      { pi_plus,  "#pi^{+}" },
      { pi_minus, "#pi^{-}" },
      { proton,   "p"       }
    };

    /// Particle mass in GeV
    const std::unordered_map<PDG, double> mass{
      { electron, 0.00051099895000 },
      { pi_plus,  0.13957039       },
      { pi_minus, 0.13957039       },
      { proton,   0.93827208816    }
    };

    // clang-format on
  }

}
