#include "ZVertexFilter.h"

namespace iguana::clas12 {

  REGISTER_IGUANA_ALGORITHM(ZVertexFilter);

  void ZVertexFilter::Start(hipo::banklist& banks)
  {

    // FIXME: need a way of passing in run numbers and pid values
    int runnb = 10000; // default to RG-A fall2018 inbending for now
    int pid   = 0; // no PID needed for this filter

    // Read YAML config file with cuts for a given run number.
    ParseYAMLConfig();
    std::vector<double> z_cuts = m_yaml_config->findKeyAtRunAndPIDVector<double>("cuts", "runs", "pid", "vals", runnb, pid);

    // define options, their default values, and cache them
    CacheOption("low", z_cuts.at(0), o_zvertex_low);
    CacheOption("high", z_cuts.at(1), o_zvertex_high);

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
    // std::cout << "low " << o_zvertex_low << " high " << o_zvertex_high << std::endl;
    return (zvertex > o_zvertex_low) && (zvertex < o_zvertex_high);
  }

  void ZVertexFilter::Stop()
  {
  }

}
