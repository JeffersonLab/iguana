#pragma once

#include "services/Algorithm.h"
#include "algorithms/AlgorithmFactory.h"

namespace iguana {

  /// @brief User-level class for running a sequence of algorithms
  class AlgorithmSequence : public Algorithm {

    public:

      /// @param name the name of this sequence
      AlgorithmSequence(const std::string name="seq") : Algorithm(name) {}
      ~AlgorithmSequence() {}

      /// Create and add an algorithm to the sequence, by name.
      ///
      /// **Example**
      /// @code
      /// Add("iguana::MyAlgorithm", "my_algorithm_name");
      /// @endcode
      /// @param class_name the name of the algorithm class
      /// @param instance_name a user-specified unique name for this algorithm instance;
      ///        if not specified, `class_name` will be used
      void Add(const std::string class_name, const std::string instance_name="");

      /// Create and add an algorithm to the sequence.
      ///
      /// **Example**
      /// @code
      /// Add<iguana::MyAlgorithm>("my_algorithm_name");
      /// @endcode
      /// @param instance_name a user-specified unique name for this algorithm instance;
      ///        if not specified, the class name will be used
      template <class ALGORITHM>
        void Add(const std::string instance_name="") {
          if(instance_name=="")
            Add(std::make_unique<ALGORITHM>());
          else
            Add(std::make_unique<ALGORITHM>(instance_name));
        }

      /// Add an existing algorithm to the sequence. The `AlgorithmSequence` instance will take ownership of the algorithm
      /// (if it is an `lvalue`, you will have to `std::move` it).
      ///
      /// **Example**
      /// @code
      /// Add(std::make_unique<iguana::MyAlgorithm>("my_algorithm_name"));
      /// @endcode
      /// @param algo the algorithm
      void Add(algo_t&& algo);

      /// Get an algorithm by its instance name
      ///
      /// **Example**
      /// @code
      /// Get<iguana::MyAlgorithm>("my_algorithm_name");
      /// @endcode
      /// @param instance_name the instance name of the algorithm
      /// @return a reference to the algorithm
      template <class ALGORITHM>
        ALGORITHM* Get(const std::string instance_name) {
          if(auto it{m_algo_names.find(instance_name)}; it != m_algo_names.end())
            return dynamic_cast<ALGORITHM*>(m_sequence[it->second].get());
          m_log->Error("cannot find algorithm '{}' in sequence", instance_name);
          throw std::runtime_error("cannot Get algorithm");
        }

      /// Set an algorithm option
      /// @see `Algorithm::SetOption`
      /// @param algo_name the algorithm instance name
      /// @param key the option name
      /// @param val the option value
      void SetOption(const std::string algo_name, const std::string key, const option_t val);

      /// Set the name of this sequence
      /// @param name the new name
      void SetName(const std::string name);

      /// Print the names of the algorithms in this sequence
      /// @param level the log level of the printout
      void PrintSequence(Logger::Level level=Logger::info) const;

      /// Sequentially call each algorithm's `Start` method
      /// @see `Algorithm::Start`
      /// @param banks the list of banks
      void Start(hipo::banklist& banks) override;

      /// Sequentially call each algorithm's `Run` method
      /// @see `Algorithm::Run`
      /// @param banks the list of banks
      void Run(hipo::banklist& banks) const override;

      /// Sequentially call each algorithm's `Stop` method
      /// @see `Algorithm::Stop`
      void Stop() override;

    private:

      /// The sequence of algorithms
      std::vector<algo_t> m_sequence;

      /// Association of algorithm name to its index in the sequence
      std::unordered_map<std::string, std::vector<algo_t>::size_type> m_algo_names;

  };
}
