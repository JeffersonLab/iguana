#include "ZVertexFilter.h"

namespace iguana::clas12 {

  REGISTER_IGUANA_ALGORITHM(ZVertexFilter);

  void ZVertexFilter::Start(hipo::banklist& banks)
  {

    // FIXME: need a way of passing in run numbers and pid values
    int runnb = 5000; // default to RG-A fall2018 inbending for now

    // Read YAML config file with cuts for a given run number.
    ParseYAMLConfig();
    o_zcuts = GetOptionVector<double>("zcuts", { GetConfig()->InRange("runs", runnb), "cuts" });
    if(o_zcuts.size() != 2) {
      m_log->Error("configuration option 'zcuts' must be an array of size 2, but it is {}", PrintOptionValue("zcuts"));
      throw std::runtime_error("bad configuration");
    }

    // cache expected bank indices
    CacheBankIndex(banks, "REC::Particle", b_particle);
  }

  void ZVertexFilter::Run(hipo::banklist& banks) const
  {

    // get the banks
    auto& particleBank = GetBank(banks, b_particle, "REC::Particle");

    // dump the bank
    ShowBank(particleBank, Logger::Header("INPUT PARTICLES"));

    // filter the input bank for requested PDG code(s)
    for(int row = 0; row < particleBank.getRows(); row++)
    {
      auto zvertex = particleBank.getFloat("vz", row);
      auto accept  = Filter(zvertex);
      if(!accept)
        MaskRow(particleBank, row);
      m_log->Debug("input vz {} -- accept = {}", zvertex, accept);
    }

    // dump the modified bank
    ShowBank(particleBank, Logger::Header("OUTPUT PARTICLES"));
  }

  bool ZVertexFilter::Filter(const double zvertex) const
  {
    return (zvertex > o_zcuts[0]) && (zvertex < o_zcuts[1]);
  }

  void ZVertexFilter::Stop()
  {
  }

}
