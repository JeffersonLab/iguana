#pragma once

#include "services/Algorithm.h"

namespace iguana {

  /// Algorithm pointer type
  using algo_t = std::unique_ptr<Algorithm>;

  /// @brief User-level class for running a sequence of algorithms
  class AlgorithmSequence : public Object {

    public:

      /// @param name the name of this sequence
      AlgorithmSequence(const std::string name="seq") : Object(name) {}
      ~AlgorithmSequence() {}

      /// Add an algorithm to the sequence.
      ///
      /// **Example**
      /// @code
      /// Add<iguana::MyAlgorithm>("my_algorithm_name");
      /// @endcode
      /// @param name the name of the algorithm
      template <class ALGORITHM>
        void Add(const std::string name) {
          Add(std::make_unique<ALGORITHM>(name));
        }

      /// Add an algorithm to the sequence. The `AlgorithmSequence` instance will take ownership of the algorithm
      /// (if it is an `lvalue`, you will have to `std::move` it).
      ///
      /// **Example**
      /// @code
      /// Add(std::make_unique<iguana::MyAlgorithm>("my_algorithm_name"));
      /// @endcode
      /// @param algo the algorithm
      void Add(algo_t&& algo);


      /// Get an algorithm by name
      /// @param name the name of the algorithm
      /// @return a reference to the algorithm
      algo_t& Get(const std::string name);

      /// Set an algorithm option
      /// @see `Algorithm::SetOption`
      /// @param algo the algorithm name
      /// @param key the option name
      /// @param val the option value
      void SetOption(const std::string algo, const std::string key, const option_t val);

      /// Set the name of this sequence
      /// @param name the new name
      void SetName(const std::string name);

      /// Print the names of the algorithms in this sequence
      /// @param level the log level of the printout
      void PrintSequence(Logger::Level level=Logger::info) const;

      /// Sequentially call each algorithm's `Start` method
      /// @see `Algorithm::Start`
      /// @param banks the list of banks
      void Start(hipo::banklist& banks) const;

      /// Sequentially call each algorithm's `Run` method
      /// @see `Algorithm::Run`
      /// @param banks the list of banks
      void Run(hipo::banklist& banks) const;

      /// Sequentially call each algorithm's `Stop` method
      /// @see `Algorithm::Stop`
      void Stop() const;

    private:

      /// The sequence of algorithms
      std::vector<algo_t> m_sequence;

      /// Association of algorithm name to its index in the sequence
      std::unordered_map<std::string, std::vector<algo_t>::size_type> m_algo_names;

  };
}
