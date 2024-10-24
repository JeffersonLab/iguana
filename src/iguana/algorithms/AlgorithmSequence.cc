#include "AlgorithmSequence.h"

namespace iguana {

  REGISTER_IGUANA_ALGORITHM(AlgorithmSequence);

  void AlgorithmSequence::Start(hipo::banklist& banks)
  {
    for(auto const& algo : m_sequence)
      algo->Start(banks);
  }
  void AlgorithmSequence::Run(hipo::banklist& banks) const
  {
    for(auto const& algo : m_sequence)
      algo->Run(banks);
  }
  void AlgorithmSequence::Stop()
  {
    for(auto const& algo : m_sequence)
      algo->Stop();
  }

  void AlgorithmSequence::Add(std::string const& class_name, std::string const& instance_name)
  {
    auto algo = AlgorithmFactory::Create(class_name);
    if(algo == nullptr) {
      m_log->Error("algorithm '{}' does not exist", class_name);
      throw std::runtime_error("AlgorithmFactory cannot create non-existent algorithm");
    }
    algo->SetName(instance_name == "" ? class_name : instance_name);
    Add(std::move(algo));
  }

  void AlgorithmSequence::Add(algo_t&& algo)
  {
    auto algoName = algo->GetName();
    m_algo_names.insert({algoName, m_sequence.size()});
    // prepend sequence name to algorithm name
    algo->SetName(m_name + "|" + algoName);
    // append algorithm to the sequence
    m_sequence.push_back(std::move(algo));
    // check for duplicate algorithm name
    if(m_algo_names.size() < m_sequence.size()) {
      m_log->Error("Duplicate algorithm name '{}' detected; please make sure all of your algorithms have unique names", algoName);
      throw std::runtime_error("cannot Add algorithm");
    }
  }

  void AlgorithmSequence::SetName(std::string_view name)
  {
    // change the `m_name+"|"` prefix of each algorithm
    for(auto const& algo : m_sequence) {
      auto algoName = algo->GetName();
      if(auto pos{algoName.find("|")}; pos != algoName.npos)
        algo->SetName(std::string(name) + algoName.substr(pos));
      else
        algo->SetName(std::string(name) + "|" + algoName);
    }
    // then change the object name
    Algorithm::SetName(name);
  }

  void AlgorithmSequence::PrintSequence(Logger::Level level) const
  {
    m_log->Print(level, "algorithms in this sequence:");
    for(auto const& algo : m_sequence)
      m_log->Print(level, " - {}", algo->GetName());
  }

  void AlgorithmSequence::SetConfigFileForEachAlgorithm(std::string const& name)
  {
    for(auto const& algo : m_sequence)
      algo->SetConfigFile(name);
  }

  void AlgorithmSequence::SetConfigDirectoryForEachAlgorithm(std::string const& name)
  {
    for(auto const& algo : m_sequence)
      algo->SetConfigDirectory(name);
  }

  void AlgorithmSequence::ForEachAlgorithm(std::function<void(algo_t&)> func)
  {
    for(auto& algo : m_sequence)
      func(algo);
  }

}
