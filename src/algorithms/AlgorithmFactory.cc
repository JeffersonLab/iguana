#include "AlgorithmFactory.h"

namespace iguana {

  std::unordered_map<std::string, AlgorithmFactory::algo_creator_t> AlgorithmFactory::s_creators;

  bool AlgorithmFactory::Register(const std::string& name, algo_creator_t creator) noexcept {
    if(auto it=s_creators.find(name); it==s_creators.end()) {
      s_creators.insert({name, creator});
      return true;
    }
    return false;
  }

  std::unique_ptr<Algorithm> AlgorithmFactory::Create(const std::string& name) noexcept {
    if(auto it=s_creators.find(name); it!=s_creators.end())
      return it->second();
    return nullptr;
  }

}
