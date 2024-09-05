#include "Algorithm.h"

namespace iguana::clas12 {

  REGISTER_IGUANA_ALGORITHM(ZVertexFilter);

  void ZVertexFilter::Start(hipo::banklist& banks)
  {

    // Read YAML config file with cuts for a given run number.
    ParseYAMLConfig();
    // o_runnum.Save(0, 0);
    // o_zcuts  = GetOptionVector<double>("cuts", {GetConfig()->InRange("runs", o_runnum), "cuts"});
    // o_pids  =  GetOptionSet<int>("pids", {GetConfig()->InRange("runs", o_runnum), "pids"});
    // if(o_zcuts.size() != 2) {
    //   m_log->Error("configuration option 'cuts' must be an array of size 2, but it is {}", PrintOptionValue("cuts"));
    //   throw std::runtime_error("bad configuration");
    // }
    // if(o_pids.size() < 1) {
    //   m_log->Error("configuration option 'pids' must have at least one value");
    //   throw std::runtime_error("bad configuration");
    // }

    o_runnum = ConcurrentParamFactory::Create<int>();

    // get expected bank indices
    b_particle = GetBankIndex(banks, "REC::Particle");
    b_config = GetBankIndex(banks, "RUN::config");
  }

  void ZVertexFilter::Run(hipo::banklist& banks) const
  {

    // get the banks
    auto& particleBank = GetBank(banks, b_particle, "REC::Particle");
    auto& configBank = GetBank(banks, b_config, "RUN::config");

    // dump the bank
    ShowBank(particleBank, Logger::Header("INPUT PARTICLES"));

    // prepare the event, reloading configuration parameters, if necessary
    auto key = PrepareEvent(configBank.getInt("run", 0));

    // filter the input bank for requested PDG code(s)
    particleBank.getMutableRowList().filter([this, &key](auto bank, auto row) {
        auto zvertex = bank.getFloat("vz", row);
        auto pid = bank.getInt("pid", row);
        auto status = bank.getShort("status", row);
        auto accept  = Filter(zvertex,pid,status,key);
        m_log->Debug("input vz {} pid {} status {} -- accept = {}", zvertex, pid, status, accept);
        return accept ? 1 : 0;
        });

    // dump the modified bank
    ShowBank(particleBank, Logger::Header("OUTPUT PARTICLES"));
  }

  concurrent_key_t ZVertexFilter::PrepareEvent(int const runnum, concurrent_key_t key) const {
    if(o_runnum->NeedsHashing()) {
      std::hash<int> hash_ftn;
      auto hash_key = hash_ftn(runnum);
      if(!o_runnum->HasKey(hash_key))
        Reload(runnum, hash_key);
      return hash_key;
    }
    if(o_runnum->Load(key) != runnum)
      Reload(runnum, key);
    return key;
  }

  void ZVertexFilter::Reload(int const runnum, concurrent_key_t key) const {
    std::lock_guard<std::mutex> const lock(m_mutex); // NOTE: be sure to lock successive `ConcurrentParam::Save` calls !!!
    o_runnum->Save(runnum, key);
    o_zcuts->Save(GetOptionVector<double>("cuts", {GetConfig()->InRange("runs", runnum), "cuts"}));
    // FIXME: handle PIDs
  }

  bool ZVertexFilter::Filter(double const zvertex, int const pid, int const status, concurrent_key_t key) const
  {
    return false; // TODO

    // auto zcuts = o_zcuts.Load(key);
    // return zvertex > zcuts.at(0) && zvertex < zcuts.at(1);

    // //only apply filter if particle pid is in list of pids
    // //and particles not in FT (ie FD or CD)
    // if(o_pids.find(pid) != o_pids.end() && abs(status)>=2000) {
    //   return zvertex > GetZcutLower() && zvertex < GetZcutUpper();
    // } else {
    //   return true;
    // }
  }

  int ZVertexFilter::GetRunNum() const
  {
    return 0; // FIXME
  }

  double ZVertexFilter::GetZcutLower() const
  {
    return 0.0; // FIXME
  }

  double ZVertexFilter::GetZcutUpper() const
  {
    return 0.0; // FIXME
  }

  void ZVertexFilter::Stop()
  {
  }

}
