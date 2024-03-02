#include "Validator.h"

namespace iguana {

  std::unordered_map<std::string, ValidatorFactory::validator_creator_t> ValidatorFactory::s_creators;

  bool ValidatorFactory::Register(const std::string& name, validator_creator_t creator) noexcept
  {
    if(auto it = s_creators.find(name); it == s_creators.end()) {
      s_creators.insert({name, creator});
      return true;
    }
    return false;
  }

  validator_t ValidatorFactory::Create(const std::string& name) noexcept
  {
    if(auto it = s_creators.find(name); it != s_creators.end())
      return it->second();
    return nullptr;
  }

}
