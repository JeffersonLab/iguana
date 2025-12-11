#include "AlgorithmSequence.h"

namespace iguana {

  REGISTER_IGUANA_ALGORITHM(AlgorithmSequence);

  void AlgorithmSequence::Start(hipo::banklist& banks)
  {
    for(auto const& algo : m_sequence)
      algo->Start(banks);
  }

  bool AlgorithmSequence::Run(hipo::banklist& banks) const
  {
    for(auto const& algo : m_sequence) {
      if(!algo->Run(banks))
        return false;
    }
    return true;
  }

  void AlgorithmSequence::Stop()
  {
    for(auto const& algo : m_sequence)
      algo->Stop();
  }

  void AlgorithmSequence::Add(std::string const& algo_class_name, std::string const& algo_instance_name)
  {
    auto algo = AlgorithmFactory::Create(algo_class_name);
    if(algo == nullptr) {
      m_log->Error("algorithm '{}' does not exist", algo_class_name);
      throw std::runtime_error("AlgorithmFactory cannot create non-existent algorithm");
    }
    algo->SetName(algo_instance_name == "" ? algo_class_name : algo_instance_name);
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

  std::vector<std::string> AlgorithmSequence::GetCreatedBankNames(std::string const& algo_instance_name) const noexcept(false)
  {
    if(auto it{m_algo_names.find(algo_instance_name)}; it != m_algo_names.end())
      return m_sequence[it->second]->GetCreatedBankNames();
    m_log->Error("cannot find algorithm '{}' in sequence", algo_instance_name);
    throw std::runtime_error("GetCreatedBankNames failed");
  }

  std::string AlgorithmSequence::GetCreatedBankName(std::string const& algo_instance_name) const noexcept(false)
  {
    if(auto it{m_algo_names.find(algo_instance_name)}; it != m_algo_names.end())
      return m_sequence[it->second]->GetCreatedBankName();
    m_log->Error("cannot find algorithm '{}' in sequence", algo_instance_name);
    throw std::runtime_error("GetCreatedBankName failed");
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

  hipo::banklist::size_type AlgorithmSequence::GetBankIndex(
      hipo::banklist& banks,
      std::string const& bank_name,
      std::string const& algo_instance_name) const noexcept(false)
  {
    if(auto it{m_algo_names.find(algo_instance_name)}; it != m_algo_names.end())
      return m_sequence.at(it->second)->GetBankIndex(banks, bank_name);
    m_log->Error("cannot find algorithm '{}' in sequence", algo_instance_name);
    throw std::runtime_error("cannot Get algorithm");
  }

  hipo::banklist::size_type AlgorithmSequence::GetCreatedBankIndex(
      hipo::banklist& banks,
      std::string const& algo_instance_name) const noexcept(false)
  {
    return GetBankIndex(banks, GetCreatedBankName(algo_instance_name), algo_instance_name);
  }

}
