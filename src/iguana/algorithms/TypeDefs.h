/// @file
/// @brief Type definitions for common objects used in algorithms

#include <tuple>

namespace iguana {

  /// Lorentz vector element type, matching that of `REC::Particle` momentum components
  using lorentz_element_t = float;

  /// Generic Lorentz vector container type
  using lorentz_vector_t = std::tuple<lorentz_element_t, lorentz_element_t, lorentz_element_t, lorentz_element_t>;

}
