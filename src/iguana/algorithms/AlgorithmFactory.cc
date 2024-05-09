#include "Algorithm.h"

namespace iguana {

  std::unordered_map<std::string, AlgorithmFactory::algo_creator_t> AlgorithmFactory::s_creators;
  std::unordered_map<std::string, std::vector<std::string>> AlgorithmFactory::s_created_banks;

  bool AlgorithmFactory::Register(std::string const& name, algo_creator_t creator, std::vector<std::string> const new_banks) noexcept
  {
    if(auto it = s_creators.find(name); it == s_creators.end()) {
      s_creators.insert({name, creator});
      for(auto const& new_bank : new_banks) {
        if(auto it = s_created_banks.find(new_bank); it == s_created_banks.end())
          s_created_banks.insert({new_bank, {}});
        s_created_banks.at(new_bank).push_back(name);
      }
      return true;
    }
    return false;
  }

  algo_t AlgorithmFactory::Create(std::string const& name)
  {
    if(auto it = s_creators.find(name); it != s_creators.end())
      return it->second();
    throw std::runtime_error(fmt::format("AlgorithmFactory: algorithm with name {:?} does not exist", name));
  }

  std::optional<std::vector<std::string>> AlgorithmFactory::QueryNewBank(std::string const& bank_name) noexcept
  {
    if(auto it = s_created_banks.find(bank_name); it != s_created_banks.end())
      return it->second;
    return {};
  }
}
