#include "Algorithm.h"

namespace iguana::clas12 {

  REGISTER_IGUANA_ALGORITHM(EventBuilderFilter);

  void EventBuilderFilter::ConfigHook()
  {
    o_particle_bank = GetOptionScalar<std::string>({"particle_bank"});
    o_pids          = GetOptionSet<int>({"pids"});
  }

  void EventBuilderFilter::StartHook(hipo::banklist& banks)
  {
    // get expected bank indices
    b_particle = GetBankIndex(banks, o_particle_bank);
  }


  bool EventBuilderFilter::RunHook(hipo::banklist& banks) const
  {
    return Run(GetBank(banks, b_particle, o_particle_bank));
  }

  bool EventBuilderFilter::Run(hipo::bank& particleBank) const
  {
    // dump the bank
    ShowBank(particleBank, Logger::Header("INPUT PARTICLES"));

    // filter the input bank for requested PDG code(s)
    particleBank.getMutableRowList().filter([this](auto bank, auto row) {
      auto pid    = bank.getInt("pid", row);
      auto accept = Filter(pid);
      m_log->Debug("input PID {} -- accept = {}", pid, accept);
      return accept ? 1 : 0;
    });

    // dump the modified bank
    ShowBank(particleBank, Logger::Header("OUTPUT PARTICLES"));

    // return false if everything is filtered out
    return !particleBank.getRowList().empty();
  }


  bool EventBuilderFilter::Filter(int const pid) const
  {
    return o_pids.find(pid) != o_pids.end();
  }

  std::deque<bool> EventBuilderFilter::Filter(std::vector<int> const pids) const
  {
    std::deque<bool> result;
    for(auto const& pid : pids)
      result.push_back(Filter(pid));
    return result;
  }

}
