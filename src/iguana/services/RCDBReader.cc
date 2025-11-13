#include "RCDBReader.h"
#include "GlobalParam.h"
// #include "iguana/algorithms/TypeDefs.h" // DEBUG ODR

#ifdef USE_RCDB
// include the RCDB headers ONCE, to avoid ODR violations
#include <RCDB/Connection.h>
#endif

namespace iguana {

  RCDBReader::RCDBReader(std::string_view name, Logger::Level lev)
      : Object(name, lev)
  {
#ifdef USE_RCDB

    // choose the RCDB URL, from the following priority ordering
    // 1. try `GlobalRcdbUrl`
    m_url = GlobalRcdbUrl();
    if(!m_url.empty())
      m_log->Debug("RCDB URL set from 'iguana::GlobalRcdbUrl': {:?}", m_url);
    else {
      // 2. try env var `RCDB_CONNECTION`
      if(auto url_ptr{std::getenv("RCDB_CONNECTION")}; url_ptr != nullptr)
        m_url = std::string(url_ptr);
      if(!m_url.empty())
        m_log->Debug("RCDB URL set from env var 'RCDB_CONNECTION': {:?}", m_url);
      else {
        // 3. fallback to default value
        m_log->Warn("RCDB URL not set; you may choose a URL with the environment variable 'RCDB_CONNECTION' or with the global parameter 'iguana::GlobalRcdbUrl'; for now, let's proceed with the URL set to {:?}", m_default_url);
        m_url = m_default_url;
        m_log->Debug("RCDB URL set from default fallback: {:?}", m_url);
      }
    }

    // then start the connection
    m_rcdb_connection = std::make_unique<rcdb::Connection>(m_url, true);

#endif
  }

  //////////////////////////////////////////////////////////////////////////////////

  RCDBReader::~RCDBReader() = default;

  //////////////////////////////////////////////////////////////////////////////////

  double RCDBReader::GetBeamEnergy(int const runnum)
  {
    double const default_value = 10.6;
    // if override value is set, return it
    if(m_beam_energy_override >= 0)
      return m_beam_energy_override;
    // if it's an MC run (pre real-run numbers), return the override value
    if(runnum == 11/*MC_RUN_NUM*/) {
      if(m_beam_energy_override >= 0)
        return m_beam_energy_override;
      else {
        std::call_once(m_error_once, [&]() { m_log->Error("Run number is {}; call `RCDBReader::SetBeamEnergyOverride` to set the beam energy (you can use `GetRCDBReader()` to get an algorithm's `RCDBReader` instance); for now, assuming it is {} GeV", runnum, default_value); });
        return default_value;
      }
    }
    // otherwise, query the RCDB
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

  //////////////////////////////////////////////////////////////////////////////////

  void RCDBReader::SetBeamEnergyOverride(double const beam_energy)
  {
    m_beam_energy_override = beam_energy;
  }

}
