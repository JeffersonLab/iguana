#pragma once

#include "Object.h"

namespace iguana {

  /// @brief RCDB reader
  class RCDBReader : public Object
  {

    public:

      /// @param name the name of this reader
      RCDBReader(std::string_view name = "rcdb");

  };
}
