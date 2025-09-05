#include "Algorithm.h"

namespace iguana {

  std::unordered_map<std::string, AlgorithmFactory::algo_creator_t> AlgorithmFactory::s_creators;
  std::unordered_map<std::string, std::vector<std::string>> AlgorithmFactory::s_bank_to_algos;
  std::unordered_map<std::string, std::vector<std::string>> AlgorithmFactory::s_algo_to_banks;

  bool AlgorithmFactory::Register(std::string const& algo_name, algo_creator_t creator, std::vector<std::string> const new_banks) noexcept
  {
    if(auto it = s_creators.find(algo_name); it == s_creators.end()) {
      s_creators.insert({algo_name, creator});
      s_algo_to_banks.insert({algo_name, new_banks});
      for(auto const& new_bank : new_banks) {
        if(auto it = s_bank_to_algos.find(new_bank); it == s_bank_to_algos.end())
          s_bank_to_algos.insert({new_bank, {}});
        s_bank_to_algos.at(new_bank).push_back(algo_name);
      }
      return true;
    }
    return false;
  }

  algo_t AlgorithmFactory::Create(std::string const& algo_name)
  {
    if(auto it = s_creators.find(algo_name); it != s_creators.end())
      return it->second();
    throw std::runtime_error(fmt::format("AlgorithmFactory: algorithm with name {:?} does not exist", algo_name));
  }

  std::optional<std::vector<std::string>> AlgorithmFactory::GetCreatorAlgorithms(std::string const& bank_name) noexcept
  {
    if(auto it = s_bank_to_algos.find(bank_name); it != s_bank_to_algos.end())
      return it->second;
    return {};
  }

  std::optional<std::vector<std::string>> AlgorithmFactory::GetCreatedBanks(std::string const& algo_name) noexcept(false)
  {
    if(auto it = s_algo_to_banks.find(algo_name); it != s_algo_to_banks.end())
      return it->second;
    return {};
  }

}
