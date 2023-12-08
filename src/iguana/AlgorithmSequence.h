#pragma once

#include "services/Algorithm.h"

namespace iguana {

  /// algorithm pointer type
  using algo_t = std::unique_ptr<Algorithm>;

  /// @brief User-level class for running a sequence of algorithms
  class AlgorithmSequence : public Object {

    public:

      /// @param name the name of this sequence; if specified, all algorithms
      ///             in this sequence will be prefixed by this name
      AlgorithmSequence(const std::string name="") : Object(name) {}
      ~AlgorithmSequence() {}

      void Add(const algo_t& algo);
      void Add(const std::vector<algo_t>& algos);

      /// get an algorithm by name
      algo_t& Get(const std::string name);

      void SetOption(const std::string algo, const std::string key, const option_t val);

      void PrintSequence(Logger::Level level=Logger::info) const;

      void Start(hipo::banklist& banks) const;
      void Run(hipo::banklist& banks) const;
      void Stop() const;

    private:

      /// Cache the algorithm names
      void CacheNames();

      /// the sequence of algorithms
      std::vector<algo_t> m_sequence;

      /// association of algorithm name to its index in the sequence
      std::unordered_map<std::string, std::vector<algo_t>::size_type> m_algo_names;

  };
}
