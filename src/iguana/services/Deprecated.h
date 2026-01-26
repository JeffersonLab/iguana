#pragma once

#include <string>

namespace iguana::deprecated {

  /// @brief check if a configuration parameter name (key), used in `Algorithm::SetOption`, has been deprecated; if so, throw exception with guidance
  /// @param algo_class_name the name of the algorithm class
  /// @param key the configuration parameter name (key) used in `Algorithm::SetOption`
  void CheckSetOptionKey(std::string const& algo_class_name, std::string const& key) noexcept(false);

}
