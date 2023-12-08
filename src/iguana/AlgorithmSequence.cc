#include "AlgorithmSequence.h"

namespace iguana {

  void AlgorithmSequence::Add(const algo_t& algo) {
    Add({std::move(algo)});
  }

  void AlgorithmSequence::Add(const std::vector<algo_t>& algos) {
    if(m_name != "") {
      for(const auto& algo : algos)
        algo->SetName(m_name + ":" + algo->GetName());
    }
    if(m_sequence.empty())
      m_sequence = std::move(algos);
    else {
      m_sequence.reserve(m_sequence.size() + algos.size());
      std::move(algos.begin(), algos.end(), std::back_inserter(m_sequence));
    }
    CacheNames();
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

  void AlgorithmSequence::PrintSequence(Logger::Level level) const {
    for(const auto& algo : m_sequence)
      m_log->Print(level, " - {}", algo->GetName());
  }

  void AlgorithmSequence::CacheNames() {
    m_algo_names.clear();
    for(std::vector<algo_t>::size_type i=0; i<m_sequence.size(); i++)
      m_algo_names.insert({m_sequence.at(i)->GetName(), i});
    if(m_algo_names.size() < m_sequence.size()) {
      m_log->Error("Duplicate algorithm name detected; please make sure all of your algorithms have unique names");
      m_log->Error("Your sequence of algorithms is:");
      PrintSequence(Logger::error);
      throw std::runtime_error("cannot configure this sequence");
    }
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
