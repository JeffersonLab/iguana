/// @file
/// @brief Type definitions for common objects used in algorithms

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

    const std::unordered_map<PDG, std::string> name{
      { electron, "electron" },
      { pi_plus,  "pi_plus"  },
      { pi_minus, "pi_minus" },
      { proton,   "proton"   }
    };

    const std::unordered_map<PDG, std::string> title{
      { electron, "e^{-}"   },
      { pi_plus,  "#pi^{+}" },
      { pi_minus, "#pi^{-}" },
      { proton,   "p"       }
    };

    // clang-format on
  }

}
