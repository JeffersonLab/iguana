#pragma once

#include <mutex>
#include <string>

#include "Object.h"

namespace iguana {

  /// @brief a globally accessible parameter
  ///
  /// A global parameter has the following properties
  /// - a default value
  /// - may be changed only _one time_
  /// - may be read from _anywhere_
  ///
  /// @par Available global parameters
  /// - `iguana::GlobalConcurrencyModel`
  template <typename T>
  class GlobalParam : public Object {

    public:

      /// @param val the initial value of this parameter
      GlobalParam(T val) : Object("IGUANA"), m_val(val) {}

      /// @brief assign a new value to this parameter
      /// @warning this may _only_ be used one time; a second attempt to set the parameter will fail
      /// @param val the new value of this parameter
      /// @returns `*this`
      GlobalParam<T>& operator=(T const& val)
      {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::call_once(m_once, [&]() { m_val = val; });
        return *this;
      }

      /// @brief get the value of the parameter
      /// @returns the value of the parameter
      T const operator()()
      {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_val;
      }

    private:

      T m_val;
      std::once_flag m_once;
      std::mutex m_mutex;

  };

  // ==================================================================================
  // IGUANA GLOBAL PARAMETERS (see source file 'GlobalParam.cc' for their default values)
  // ==================================================================================

  /// @brief The concurrency model, for running certain algorithms in a thread-safe way
  /// @par Available Models
  /// - "single": no thread safety, but optimal for single-threaded users
  /// - "memoize": thread-safe lazy loading of configuration parameters
  /// - "none": no concurrency model set by user; this is the *default option*, and if this
  ///   is the choice when `ConcurrentParamFactory::Create` is called, an appropriate
  ///   option will be _chosen_ by `ConcurrentParamFactory::Create` instead
  extern GlobalParam<std::string> GlobalConcurrencyModel;

  /// @brief RCDB URL
  /// @par Notes
  /// The RCDB will check for a URL in the following order:
  /// - This parameter, `iguana::GlobalRcdbUrl`; by default it is not set to any value
  /// - The environment variable `RCDB_CONNECTION` (which is likely set if you are on `ifarm`)
  /// - A default URL, which will be printed in a warning; see `iguana::RCDBReader::m_default_url`
  extern GlobalParam<std::string> GlobalRcdbUrl;

}
