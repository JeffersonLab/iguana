#include "Validator.h"

namespace iguana {

  void Validator::SetOutputDirectory(std::string_view output_dir)
  {
    m_output_dir = output_dir;
  }

  std::optional<std::string> Validator::GetOutputDirectory()
  {
    if(m_output_dir != "")
      return m_output_dir;
    return {};
  }

}
