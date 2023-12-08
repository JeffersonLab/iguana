#include "AlgorithmSequence.h"

namespace iguana {

  void AlgorithmSequence::Add(algo_t&& algo) {
    m_algo_names.insert({algo->GetName(), m_sequence.size()});
    algo->SetName(m_name + "|" + algo->GetName()); // prepend sequence name to algorithm name
    m_sequence.push_back(std::move(algo));
    if(m_algo_names.size() < m_sequence.size()) { // check for duplicate algorithm name
      m_log->Error("Duplicate algorithm name detected; please make sure all of your algorithms have unique names");
      m_log->Error("Your sequence of algorithms is:");
      PrintSequence(Logger::error);
      throw std::runtime_error("cannot configure this sequence");
    }
  }

  algo_t& AlgorithmSequence::Get(const std::string name) {
    if(auto it{m_algo_names.find(name)}; it != m_algo_names.end()) {
      auto& algo = m_sequence[it->second];
      return algo;
    }
    else
      throw std::runtime_error(fmt::format("cannot find algorithm '{}' in sequence", name));
  }

  void AlgorithmSequence::SetOption(const std::string algo, const std::string key, const option_t val) {
    Get(algo)->SetOption(key,val);
  }

  void AlgorithmSequence::SetName(const std::string name) {
    // change the `m_name+"|"` prefix of each algorithm
    for(const auto& algo : m_sequence) {
      auto algoName = algo->GetName();
      if(auto pos{algoName.find("|")}; pos != algoName.npos)
        algo->SetName(name + algoName.substr(pos));
      else
        algo->SetName(name + "|" + algoName);
    }
    // then change the object name
    Object::SetName(name);
  }

  void AlgorithmSequence::PrintSequence(Logger::Level level) const {
    m_log->Print(level, "algorithms in this sequence:");
    for(const auto& algo : m_sequence)
      m_log->Print(level, " - {}", algo->GetName());
  }

  void AlgorithmSequence::Start(hipo::banklist& banks) const {
    for(const auto& algo : m_sequence)
      algo->Start(banks);
  }

  void AlgorithmSequence::Run(hipo::banklist& banks) const {
    for(const auto& algo : m_sequence)
      algo->Run(banks);
  }

  void AlgorithmSequence::Stop() const {
    for(const auto& algo : m_sequence)
      algo->Stop();
  }

}
