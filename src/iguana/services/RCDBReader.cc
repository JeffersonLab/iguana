#include "RCDBReader.h"

namespace iguana {

  RCDBReader::RCDBReader(std::string_view name) : Object(name)
  {
#ifdef USE_RCDB
    auto url_ptr = std::getenv("RCDB_CONNECTION");
    m_url = url_ptr != nullptr ? std::string(url_ptr) : default_url;
    m_log->Debug("RCDB URL: {}", m_url);
    m_rcdb_connection = std::make_unique<rcdb::Connection>(m_url, true);
#endif
  }

  double RCDBReader::GetBeamEnergy(int const runnum)
  {
    double const default_value = 10.6;
#ifdef USE_RCDB
    auto cnd = m_rcdb_connection->GetCondition(runnum, "beam_energy");
    if(!cnd) {
      m_log->Error("Failed to find beam energy from RCDB for run {}; assuming it is {} GeV", runnum, default_value);
      return default_value;
    }
    return cnd->ToDouble() / 1e3; // convert [MeV] -> [GeV]
#else
    std::call_once(m_error_once, [&]() { m_log->Error("RCDB dependency not found; RCDBReader::GetBeamEnergy will return the default value of {} GeV.", default_value); });
    return default_value;
#endif
  }

}
