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
      RCDBReader(std::string_view name = "rcdb");

      /// @param runnum run number
      /// @returns the beam energy in GeV
      double GetBeamEnergy(int const runnum);

    private:

      std::string m_url;
      std::string const default_url = "mysql://rcdb@clasdb.jlab.org/rcdb";
      std::once_flag m_error_once;

#ifdef USE_RCDB
      std::unique_ptr<rcdb::Connection> m_rcdb_connection;
#endif

  };
}
