#pragma once

#include "Object.h"
#include <mutex>

#ifdef USE_RCDB
#include <RCDB/Connection.h>
#endif

namespace iguana {

  /// @brief RCDB reader
  ///
  /// This class interfaces to the RCDB. The database connection path is chosen from one of the following, in order:
  /// - The global variable `iguana::GlobalRcdbUrl`; by default this is not set to any value (its type is `iguana::GlobalParam`)
  /// - The environment variable `RCDB_CONNECTION` (which is likely set if you are on `ifarm`)
  /// - A default URL, which will be printed in a warning; see `iguana::RCDBReader::m_default_url`
  ///
  /// RCDB will automatically use `mariadb` / `mysql` or `sqlite`, depending on the RCDB path, and whether you have satisfied the dependencies.
  class RCDBReader : public Object
  {

    public:

      /// @param name the name of this reader
      /// @param lev the log level
      RCDBReader(std::string_view name = "rcdb", Logger::Level lev = Logger::DEFAULT_LEVEL);

      /// @param runnum run number
      /// @returns the beam energy in GeV
      double GetBeamEnergy(int const runnum);

    protected:

      /// @brief default RCDB URL, used as a last resort
      std::string const m_default_url = "mysql://rcdb@clasdb.jlab.org/rcdb";

    private:

      std::string m_url;
      std::once_flag m_error_once;

#ifdef USE_RCDB
      std::unique_ptr<rcdb::Connection> m_rcdb_connection;
#endif

  };
}
