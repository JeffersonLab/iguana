#include "Algorithm.h"
#include "iguana/algorithms/TypeDefs.h"

namespace iguana::clas12 {

  REGISTER_IGUANA_ALGORITHM(ZVertexFilter);

  void ZVertexFilter::Start(hipo::banklist& banks)
  {

    // get configuration
    ParseYAMLConfig();
    o_particle_bank    = GetOptionScalar<std::string>("particle_bank");
    o_runnum           = ConcurrentParamFactory::Create<int>();
    o_electron_vz_cuts = ConcurrentParamFactory::Create<std::vector<double>>();

    // get expected bank indices
    b_particle = GetBankIndex(banks, o_particle_bank);
    b_config   = GetBankIndex(banks, "RUN::config");
  }

  bool ZVertexFilter::Run(hipo::banklist& banks) const
  {
    return Run(
        GetBank(banks, b_particle, o_particle_bank),
        GetBank(banks, b_config, "RUN::config"));
  }

  bool ZVertexFilter::Run(hipo::bank& particleBank, hipo::bank const& configBank) const
  {
    // dump the bank
    ShowBank(particleBank, Logger::Header("INPUT PARTICLES"));

    // prepare the event, reloading configuration parameters, if necessary
    auto key = PrepareEvent(configBank.getInt("run", 0));

    // filter the input bank for requested PDG code(s)
    particleBank.getMutableRowList().filter([this, key](auto bank, auto row) {
      auto zvertex = bank.getFloat("vz", row);
      auto pid     = bank.getInt("pid", row);
      auto status  = bank.getShort("status", row);
      auto accept  = Filter(zvertex, pid, status, key);
      m_log->Debug("input vz {} pid {} status {} -- accept = {}", zvertex, pid, status, accept);
      return accept ? 1 : 0;
    });

    // dump the modified bank
    ShowBank(particleBank, Logger::Header("OUTPUT PARTICLES"));
    return !particleBank.getRowList().empty();
  }

  concurrent_key_t ZVertexFilter::PrepareEvent(int const runnum) const
  {
    m_log->Trace("calling PrepareEvent({})", runnum);
    if(o_runnum->NeedsHashing()) {
      std::hash<int> hash_ftn;
      auto hash_key = hash_ftn(runnum);
      if(!o_runnum->HasKey(hash_key))
        Reload(runnum, hash_key);
      return hash_key;
    }
    else {
      if(o_runnum->IsEmpty() || o_runnum->Load(0) != runnum)
        Reload(runnum, 0);
      return 0;
    }
  }

  void ZVertexFilter::Reload(int const runnum, concurrent_key_t key) const
  {
    std::lock_guard<std::mutex> const lock(m_mutex); // NOTE: be sure to lock successive `ConcurrentParam::Save` calls !!!
    m_log->Trace("-> calling Reload({}, {})", runnum, key);
    o_runnum->Save(runnum, key);
    o_electron_vz_cuts->Save(GetOptionVector<double>("electron_vz", {"electron", GetConfig()->InRange("runs", runnum), "vz"}), key);
  }

  bool ZVertexFilter::Filter(double const zvertex, int const pid, int const status, concurrent_key_t key) const
  {
    if(pid == particle::PDG::electron && abs(status) >= 2000) {
      auto zcuts = GetElectronZcuts(key);
      return zvertex > zcuts.at(0) && zvertex < zcuts.at(1);
    }
    return true; // cuts don't apply
  }

  int ZVertexFilter::GetRunNum(concurrent_key_t const key) const
  {
    return o_runnum->Load(key);
  }

  std::vector<double> ZVertexFilter::GetElectronZcuts(concurrent_key_t const key) const
  {
    return o_electron_vz_cuts->Load(key);
  }

  void ZVertexFilter::SetElectronZcuts(double zcut_lower, double zcut_upper, concurrent_key_t const key)
  {
    o_electron_vz_cuts->Save({zcut_lower, zcut_upper}, key);
  }

  void ZVertexFilter::Stop()
  {
  }

}
