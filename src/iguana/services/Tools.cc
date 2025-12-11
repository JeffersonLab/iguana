#include "Tools.h"
#include <stdexcept>

namespace iguana::tools {

  std::string ExpandTilde(std::string const& path)
  {
    if(path.empty() || path[0] != '~')
      return path;
    if(char const* home_dir = std::getenv("HOME"); home_dir)
      return std::string(home_dir) + path.substr(1);
    throw std::runtime_error("cannot expand `~` since $HOME is not set");
  }

}
