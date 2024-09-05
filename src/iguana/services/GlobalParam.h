#pragma once

#include <mutex>
#include <string>

#include "Object.h"

namespace iguana {

  /// @brief a globally accessible parameter
  ///
  /// A global parameter:
  /// - has a default value
  /// - may be changed _once_
  /// - may be read from anywhere
  template <typename T>
  class GlobalParam : public Object {

    public:

      /// @param val the initial value of this parameter
      GlobalParam(T val) : Object("IGUANA"), m_val(val) {}

      /// @brief assign a new value to this parameter
      /// @warning this may _only_ used one time; a second attempt to set the parameter will fail
      /// @param val the new value of this parameter
      /// @returns `*this`
      GlobalParam<T>& operator=(T const& val)
      {
        std::call_once(m_once, [&]() { m_val = val; });
        return *this;
      }

      /// @brief get the value of the parameter
      /// @returns the value of the parameter
      T const& operator()()
      {
        return m_val;
      }

    private:

      T m_val;
      std::once_flag m_once;

  };

  // ==================================================================================
  // IGUANA GLOBAL PARAMETERS (see source file 'GlobalParam.cc' for their default values)
  // ==================================================================================

  /// the concurrency model, for running certain algorithms in a thread-safe way
  extern GlobalParam<std::string> GlobalConcurrencyModel;

}
