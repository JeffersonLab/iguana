#include "Algorithm.h"

namespace iguana {

  std::unordered_map<std::string, AlgorithmFactory::algo_creator_t> AlgorithmFactory::s_creators;
  std::unordered_map<std::string, std::vector<std::string>> AlgorithmFactory::s_created_banks;

  bool AlgorithmFactory::Register(const std::string& name, algo_creator_t creator) noexcept
  {
    if(auto it = s_creators.find(name); it == s_creators.end()) {
      s_creators.insert({name, creator});
      return true;
    }
    return false;
  }

  algo_t AlgorithmFactory::Create(const std::string& name)
  {
    if(auto it = s_creators.find(name); it != s_creators.end())
      return it->second();
    throw std::runtime_error(fmt::format("AlgorithmFactory: algorithm with name {:?} does not exist", name));
  }

  bool AlgorithmFactory::RegisterNewBanks(const std::vector<std::string>& bank_names, const std::string& algo_name) noexcept
  {
    for(const auto& bank_name : bank_names) {
      if(auto it = s_created_banks.find(bank_name); it == s_created_banks.end())
        s_created_banks.insert({bank_name, {}});
      s_created_banks.at(bank_name).push_back(algo_name);
    }
    return true;
  }

  std::optional<std::vector<std::string>> AlgorithmFactory::QueryNewBank(const std::string& bank_name) noexcept
  {
    if(auto it = s_created_banks.find(bank_name); it != s_created_banks.end())
      return it->second;
    return {};
  }
}
