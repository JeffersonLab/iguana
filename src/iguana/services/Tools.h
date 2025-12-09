#pragma once

#include <string>

namespace iguana::tools {

  /// @brief expand `~` to the user's home directory
  /// @param path the file path
  /// @returns the file path with `~` expanded to `$HOME`
  std::string ExpandTilde(std::string const& path);

}
