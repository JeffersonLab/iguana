/// @file
/// @brief Type definitions for common objects used in algorithms

#include <tuple>

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

    /// PDG codes
    enum PDG {
      electron = 11,
      pi_plus  = 211,
      pi_minus = -211,
      proton   = 2212
    };

  }

}
