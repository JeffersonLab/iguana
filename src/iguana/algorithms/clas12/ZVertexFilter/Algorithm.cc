#include "Algorithm.h"

namespace iguana::clas12 {

  REGISTER_IGUANA_ALGORITHM(ZVertexFilter);

  void ZVertexFilter::Start(hipo::banklist& banks)
  {

    // Read YAML config file with cuts for a given run number.
    ParseYAMLConfig();
    o_runnum = GetCachedOption<int>("runnum").value_or(0); // FIXME: should be set form RUN::config
    o_zcuts  = GetOptionVector<double>("cuts", {GetConfig()->InRange("runs", o_runnum), "cuts"});
    o_pids  =  GetOptionSet<int>("pids", {GetConfig()->InRange("runs", o_runnum), "pids"});
    if(o_zcuts.size() != 2) {
      m_log->Error("configuration option 'cuts' must be an array of size 2, but it is {}", PrintOptionValue("cuts"));
      throw std::runtime_error("bad configuration");
    }
    if(o_pids.size() < 1) {
      m_log->Error("configuration option 'pids' must have at least one value");
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
    particleBank.getMutableRowList().filter([this](auto bank, auto row) {
        auto zvertex = bank.getFloat("vz", row);
        auto pid = bank.getInt("pid", row);
        auto status = bank.getInt("status", row);
        auto accept  = Filter(zvertex,pid,status);
        m_log->Debug("input vz {} pid {} status {} -- accept = {}", zvertex, pid, status, accept);
        return accept ? 1 : 0;
        });

    // dump the modified bank
    ShowBank(particleBank, Logger::Header("OUTPUT PARTICLES"));
  }

  bool ZVertexFilter::Filter(double const zvertex, int pid, int status) const
  {
    //only apply filter if particle pid is in list of pids
    //and particles not in FT (ie FD or CD)
    if(o_pids.find(pid) != o_pids.end() && abs(status)>=2000) {
      return zvertex > GetZcutLower() && zvertex < GetZcutUpper();
    } else {
      return true;
    }
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
