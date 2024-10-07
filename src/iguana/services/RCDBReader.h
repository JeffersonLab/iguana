#pragma once

#include "Object.h"
#include <mutex>

#ifdef USE_RCDB
#include <RCDB/Connection.h>
#endif

namespace iguana {

  /// @brief RCDB reader
  class RCDBReader : public Object
  {

    public:

      /// @param name the name of this reader
      /// @param lev the log level
      RCDBReader(std::string_view name = "rcdb", Logger::Level lev = Logger::DEFAULT_LEVEL);

      /// @param runnum run number
      /// @returns the beam energy in GeV
      double GetBeamEnergy(int const runnum);

    private:

      /// @brief default RCDB URL, used as a last resort
      std::string const m_default_url = "mysql://rcdb@clasdb.jlab.org/rcdb";

      std::string m_url;
      std::once_flag m_error_once;

#ifdef USE_RCDB
      std::unique_ptr<rcdb::Connection> m_rcdb_connection;
#endif

  };
}
