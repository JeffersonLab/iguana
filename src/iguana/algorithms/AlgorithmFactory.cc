#include "Algorithm.h"

namespace iguana {

  std::unordered_map<std::string, AlgorithmFactory::algo_creator_t> AlgorithmFactory::s_creators;
  std::unordered_map<std::string, std::vector<std::string>> AlgorithmFactory::s_created_banks;

  bool AlgorithmFactory::Register(std::string_view name, algo_creator_t creator, const std::vector<std::string> new_banks) noexcept
  {
    if(auto it = s_creators.find(name.data()); it == s_creators.end()) {
      s_creators.insert({name.data(), creator});
      for(auto const& new_bank : new_banks) {
        if(auto it = s_created_banks.find(new_bank); it == s_created_banks.end())
          s_created_banks.insert({new_bank, {}});
        s_created_banks.at(new_bank).push_back(name.data());
      }
      return true;
    }
    return false;
  }

  algo_t AlgorithmFactory::Create(std::string_view name)
  {
    if(auto it = s_creators.find(name.data()); it != s_creators.end())
      return it->second();
    throw std::runtime_error(fmt::format("AlgorithmFactory: algorithm with name {:?} does not exist", name));
  }

  std::optional<std::vector<std::string>> AlgorithmFactory::QueryNewBank(std::string_view bank_name) noexcept
  {
    if(auto it = s_created_banks.find(bank_name.data()); it != s_created_banks.end())
      return it->second;
    return {};
  }
}
