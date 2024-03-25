#include "ZVertexFilter.h"

namespace iguana::clas12 {

  REGISTER_IGUANA_ALGORITHM(ZVertexFilter);

  void ZVertexFilter::Start(hipo::banklist& banks)
  {

    // Read YAML config file with cuts for a given run number.
    ParseYAMLConfig();
    o_runnum = GetCachedOption<int>("runnum").value_or(0); // FIXME: should be set form RUN::config
    o_zcuts  = GetOptionVector<double>("zcuts", {GetConfig()->InRange("runs", o_runnum), "cuts"});
    if(o_zcuts.size() != 2) {
      m_log->Error("configuration option 'zcuts' must be an array of size 2, but it is {}", PrintOptionValue("zcuts"));
      throw std::runtime_error("bad configuration");
    }

    // get expected bank indices
    b_particle = GetBankIndex(banks, "REC::Particle");
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

  bool ZVertexFilter::Filter(double const zvertex) const
  {
    return zvertex > GetZcutLower() && zvertex < GetZcutUpper();
  }

  int ZVertexFilter::GetRunNum() const
  {
    return o_runnum;
  }

  double ZVertexFilter::GetZcutLower() const
  {
    return o_zcuts.at(0);
  }

  double ZVertexFilter::GetZcutUpper() const
  {
    return o_zcuts.at(1);
  }

  void ZVertexFilter::Stop()
  {
  }

}
