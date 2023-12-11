#pragma once

#include <memory>
#include <unordered_map>

#include "services/Algorithm.h"

namespace iguana {

  /// @brief Factory to create an algorithm.
  class AlgorithmFactory {

    public:

      /// Algorithm creator function type
      using algo_creator_t = std::unique_ptr<Algorithm>(*)();

      AlgorithmFactory() = delete;

      /// Register an algorithm with a unique name. Algorithms register themselves by calling this function.
      /// @param name the name of the algorithm (not equivalent to `Object::m_name`)
      /// @param creator the creator function
      static bool Register(const std::string& name, algo_creator_t creator) noexcept;

      /// Create an algorithm.
      /// @param name the name of the algorithm, which was used as an argument in the `AlgorithmFactory::Register` call
      static std::unique_ptr<Algorithm> Create(const std::string& name) noexcept;

    private:

      /// Association between the algorithm names and their creators
      static std::unordered_map<std::string, algo_creator_t> s_creators;

  };
}
