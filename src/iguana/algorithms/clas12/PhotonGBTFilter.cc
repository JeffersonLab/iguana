#include "PhotonGBTFilter.h"

namespace iguana::clas12 {

  REGISTER_IGUANA_ALGORITHM(PhotonGBTFilter);
  // REGISTER_IGUANA_ALGORITHM(PhotonGBTFilter , "clas12::newBank1", "clas12::newBank2"); // if this algorithm creates 2 new banks

  void PhotonGBTFilter::Start(hipo::banklist& banks)
  {
    ParseYAMLConfig();
    o_exampleInt    = GetOptionScalar<int>("exampleInt");
    o_exampleDouble = GetOptionScalar<double>("exampleDouble");
    b_particle = GetBankIndex(banks, "REC::Particle");
    // CreateBank(.....); // use this to create a new bank
  }


  void PhotonGBTFilter::Run(hipo::banklist& banks) const
  {
    auto& particleBank = GetBank(banks, b_particle, "REC::Particle");
    ShowBank(particleBank, Logger::Header("INPUT PARTICLES"));

    for(int row = 0; row < particleBank.getRows(); row++) {
      auto pid    = particleBank.getInt("pid", row);
      auto accept = Filter(pid);
      if(!accept)
        MaskRow(particleBank, row);
      m_log->Debug("input PID {} -- accept = {}", pid, accept);
    }

    ShowBank(particleBank, Logger::Header("OUTPUT PARTICLES"));
  }


  bool PhotonGBTFilter::Filter(int const pid) const
  {
    return pid > 0;
  }


  void PhotonGBTFilter::Stop()
  {
  }

}
