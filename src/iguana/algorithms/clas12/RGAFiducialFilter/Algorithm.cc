#include "Algorithm.h"

namespace iguana::clas12 {

  REGISTER_IGUANA_ALGORITHM(RGAFiducialFilter);
  // REGISTER_IGUANA_ALGORITHM(RGAFiducialFilter , "clas12::newBank1", "clas12::newBank2"); // if this algorithm creates 2 new banks

  void RGAFiducialFilter::Start(hipo::banklist& banks)
  {
    ParseYAMLConfig();
    o_exampleInt    = GetOptionScalar<int>("exampleInt");
    o_exampleDouble = GetOptionScalar<double>("exampleDouble");
    b_particle = GetBankIndex(banks, "REC::Particle");
    // CreateBank(.....); // use this to create a new bank
  }


  void RGAFiducialFilter::Run(hipo::banklist& banks) const
  {
    auto& particleBank = GetBank(banks, b_particle, "REC::Particle");
    ShowBank(particleBank, Logger::Header("INPUT PARTICLES"));

    particleBank.getMutableRowList().filter([this, &particleBank](auto bank, auto row) {
      auto pid    = particleBank.getInt("pid", row);
      auto accept = Filter(pid);
      m_log->Debug("input PID {} -- accept = {}", pid, accept);
      return accept ? 1 : 0;
      });

    ShowBank(particleBank, Logger::Header("OUTPUT PARTICLES"));
  }


  bool RGAFiducialFilter::Filter(int const pid) const
  {
    return pid > 0;
  }


  void RGAFiducialFilter::Stop()
  {
  }

}
