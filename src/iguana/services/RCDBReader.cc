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
#ifdef USE_RCDB
    auto cnd = m_rcdb_connection->GetCondition(runnum, "beam_energy");
    if(!cnd)
      throw std::runtime_error(fmt::format("Failed to find beam energy from RCDB for run {}", runnum));
    return cnd->ToDouble();
#else
    double def = 10.6;
    m_log->Warn("RCDB dependency not found; GetBeamEnergy will return default value of {} GeV.", def);
    return def;
#endif
  }

}
