#include "Algorithm.h"

namespace iguana::clas12 {

  REGISTER_IGUANA_ALGORITHM(ZVertexFilter);

  void ZVertexFilter::Start(hipo::banklist& banks)
  {

    // get configuration
    ParseYAMLConfig();
    o_runnum = ConcurrentParamFactory::Create<int>();
    o_zcuts  = ConcurrentParamFactory::Create<std::vector<double>>();
    PrepareEvent(0); // load default values (using runnum==0)

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
    m_log->Trace("calling PrepareEvent({}, {})", runnum, key);
    if(o_runnum->NeedsHashing()) {
      std::hash<int> hash_ftn;
      auto hash_key = hash_ftn(runnum);
      if(!o_runnum->HasKey(hash_key))
        Reload(runnum, hash_key);
      return hash_key;
    }
    if(o_runnum->IsEmpty() || o_runnum->Load(key) != runnum)
      Reload(runnum, key);
    return key;
  }

  void ZVertexFilter::Reload(int const runnum, concurrent_key_t key) const {
    std::lock_guard<std::mutex> const lock(m_mutex); // NOTE: be sure to lock successive `ConcurrentParam::Save` calls !!!
    m_log->Trace("-> calling Reload({}, {})", runnum, key);
    o_runnum->Save(runnum, key);
    o_zcuts->Save(GetOptionVector<double>("cuts", {GetConfig()->InRange("runs", runnum), "cuts"}), key);
    // FIXME: handle PIDs
  }

  bool ZVertexFilter::Filter(double const zvertex, int const pid, int const status, concurrent_key_t key) const
  {
    auto zcuts = GetZcuts(key);
    return zvertex > zcuts.at(0) && zvertex < zcuts.at(1);

    // FIXME
    // FIXME
    // FIXME
    // //only apply filter if particle pid is in list of pids
    // //and particles not in FT (ie FD or CD)
    // if(o_pids.find(pid) != o_pids.end() && abs(status)>=2000) {
    //   return zvertex > GetZcutLower() && zvertex < GetZcutUpper();
    // } else {
    //   return true;
    // }
  }

  int ZVertexFilter::GetRunNum(concurrent_key_t const key) const
  {
    return o_runnum->Load(key);
  }

  std::vector<double> ZVertexFilter::GetZcuts(concurrent_key_t const key) const
  {
    return o_zcuts->Load(key);
  }

  void ZVertexFilter::SetZcuts(double zcut_lower, double zcut_upper, concurrent_key_t const key)
  {
    o_zcuts->Save({zcut_lower, zcut_upper}, key);
  }

  void ZVertexFilter::Stop()
  {
  }

}
