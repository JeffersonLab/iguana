#include "Algorithm.h"

namespace iguana::clas12 {

  REGISTER_IGUANA_ALGORITHM(QADB);

  void QADB::Start(hipo::banklist& banks)
  {

    // get configuration
    ParseYAMLConfig();
    o_datasets    = GetOptionVector<std::string>("datasets");
    o_qadb_dir    = GetOptionScalar<std::string>("qadb_dir");
    o_create_bank = GetOptionScalar<bool>("create_bank");
    o_runnum      = ConcurrentParamFactory::Create<int>();
    o_binnum      = ConcurrentParamFactory::Create<int>();

    // get expected bank indices
    b_config = GetBankIndex(banks, "RUN::config");
  }

  void QADB::Run(hipo::banklist& banks) const
  {

    // get the banks
    auto& configBank = GetBank(banks, b_config, "RUN::config");

    // prepare the event, reloading configuration parameters, if necessary
    auto key = PrepareEvent(configBank.getInt("run", 0), configBank.getInt("event", 0));

    Filter(key); // FIXME: do something with this
  }

  concurrent_key_t QADB::PrepareEvent(int const runnum, int const evnum) const {
    m_log->Trace("calling PrepareEvent({}, {})", runnum, evnum);
    if(o_runnum->NeedsHashing()) {
      std::hash<int> hash_ftn;
      auto hash_key = hash_ftn(runnum); // FIXME: need to hash both runnum and evnum
      if(!o_runnum->HasKey(hash_key))
        Reload(runnum, evnum, hash_key);
      return hash_key;
    } else {
      if(o_runnum->IsEmpty() || o_runnum->Load(0) != runnum) // FIXME: bound check evnum here
        Reload(runnum, evnum, 0);
      return 0;
    }
  }

  void QADB::Reload(int const runnum, int const evnum, concurrent_key_t key) const {
    std::lock_guard<std::mutex> const lock(m_mutex); // NOTE: be sure to lock successive `ConcurrentParam::Save` calls !!!
    m_log->Trace("-> calling Reload({}, {})", runnum, key);
    o_runnum->Save(runnum, key);
    // FIXME: query QADB here
  }

  bool QADB::Filter(concurrent_key_t key) const
  {
    return true; // FIXME
  }

  int QADB::GetRunNum(concurrent_key_t const key) const
  {
    return o_runnum->Load(key);
  }

  int QADB::GetBinNum(concurrent_key_t const key) const
  {
    return o_binnum->Load(key);
  }

  void QADB::Stop()
  {
  }

}
