#include "AlgorithmSequence.h"

namespace iguana {

  REGISTER_IGUANA_ALGORITHM(AlgorithmSequence);

  void AlgorithmSequence::Start(hipo::banklist& banks) {
    for(const auto& algo : m_sequence) algo->Start(banks);
  }
  void AlgorithmSequence::Run(hipo::banklist& banks) const {
    for(const auto& algo : m_sequence) algo->Run(banks);
  }
  void AlgorithmSequence::Stop() {
    for(const auto& algo : m_sequence) algo->Stop();
  }

  void AlgorithmSequence::Add(const std::string class_name, const std::string instance_name) {
    auto algo = AlgorithmFactory::Create(class_name);
    if(algo==nullptr) {
      m_log->Error("algorithm '{}' does not exist", class_name);
      throw std::runtime_error("AlgorithmFactory cannot create non-existent algorithm");
    }
    algo->SetName(instance_name=="" ? class_name : instance_name);
    Add(std::move(algo));
  }

  void AlgorithmSequence::Add(algo_t&& algo) {
    auto algoName = algo->GetName();
    m_algo_names.insert({algoName, m_sequence.size()});
    // prepend sequence name to algorithm name
    algo->SetName(m_name + "|" + algoName);
    // use `this` config file manager in each of its algorithms (must be called AFTER `SetName`)
    algo->SetConfigFileManager(GetConfigFileManager());
    m_sequence.push_back(std::move(algo));
    // check for duplicate algorithm name
    if(m_algo_names.size() < m_sequence.size()) {
      m_log->Error("Duplicate algorithm name '{}' detected; please make sure all of your algorithms have unique names", algoName);
      throw std::runtime_error("cannot Add algorithm");
    }
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
    Algorithm::SetName(name);
  }

  void AlgorithmSequence::PrintSequence(Logger::Level level) const {
    m_log->Print(level, "algorithms in this sequence:");
    for(const auto& algo : m_sequence)
      m_log->Print(level, " - {}", algo->GetName());
  }

}
