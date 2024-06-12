#include "Algorithm.h"

namespace iguana::clas12 {

  REGISTER_IGUANA_ALGORITHM(ZVertexFilter);

  void ZVertexFilter::Start(hipo::banklist& banks)
  {

    // Read YAML config file with cuts for a given run number.
    ParseYAMLConfig();
    oa_runnum.store(0);

    // get expected bank indices
    b_particle = GetBankIndex(banks, "REC::Particle");
    b_config   = GetBankIndex(banks, "RUN::config");
  }

  void ZVertexFilter::Reload(int const& runnum) const
  {
    auto o_runnum = oa_runnum.exchange(runnum);
    if(o_runnum != runnum) {
      m_log->Debug("run number changed {} -> {}", o_runnum, runnum);
      // FIXME: `GetOptionVector` is not const
      // auto o_zcuts = GetOptionVector<double>("cuts", {GetConfig()->InRange("runs", o_runnum), "cuts"});
      // if(o_zcuts.size() != 2) {
      //   m_log->Error("configuration option 'cuts' must be an array of size 2, but it is {}", PrintOptionValue("cuts"));
      //   throw std::runtime_error("bad configuration");
      // }
      // oa_zcut_low.store(o_zcuts[0]);
      // oa_zcut_high.store(o_zcuts[1]);
    }
  }

  void ZVertexFilter::Run(hipo::banklist& banks) const
  {

    // get the banks
    auto& particleBank = GetBank(banks, b_particle, "REC::Particle");
    auto& configBank   = GetBank(banks, b_config, "RUN::config");

    // get the zcuts
    Reload(configBank.getInt("run", 0));

    // dump the bank
    ShowBank(particleBank, Logger::Header("INPUT PARTICLES"));

    //
    //
    // FIXME: would it be faster to do ONE atomic load of the z-vertex cuts here,
    // before looping over particles?
    //
    //

    // filter the input bank for requested PDG code(s)
    particleBank.getMutableRowList().filter([this](auto bank, auto row) {
        auto zvertex = bank.getFloat("vz", row);
        auto accept  = Filter(zvertex);
        m_log->Debug("input vz {} -- accept = {}", zvertex, accept);
        return accept ? 1 : 0;
        });

    // dump the modified bank
    ShowBank(particleBank, Logger::Header("OUTPUT PARTICLES"));
  }

  bool ZVertexFilter::Filter(double const zvertex) const
  {
    return zvertex > GetZcutLower() && zvertex < GetZcutUpper();
  }

  int ZVertexFilter::GetRunNum() const
  {
    return oa_runnum.load();
  }

  double ZVertexFilter::GetZcutLower() const
  {
    return oa_zcut_low.load();
  }

  double ZVertexFilter::GetZcutUpper() const
  {
    return oa_zcut_high.load();
  }

  void ZVertexFilter::Stop()
  {
  }

}
