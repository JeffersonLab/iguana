#pragma once

#include "Algorithm.h"

namespace iguana {

  /// @brief An algorithm that can run a sequence of algorithms
  ///
  /// The `Start`, `Run`, and `Stop` methods will sequentially call the corresponding algorithms' methods,
  /// in the order the algorithms were added to the sequence by `AlgorithmSequence::Add`. If an algorithm's
  /// `Run` function returns false, then `AlgorithmSequence`'s `Run` function will stop and return `false`.
  class AlgorithmSequence : public Algorithm
  {

      DEFINE_IGUANA_ALGORITHM(AlgorithmSequence, seq)

    public:

      void Start(hipo::banklist& banks) override;
      bool Run(hipo::banklist& banks) const override;
      void Stop() override;

      /// Create and add an algorithm to the sequence, by name.
      ///
      /// **Example**
      /// @code
      /// Add("iguana::MyAlgorithm", "my_algorithm_name");
      /// @endcode
      /// @param class_name the name of the algorithm class
      /// @param instance_name a user-specified unique name for this algorithm instance;
      ///        if not specified, `class_name` will be used
      void Add(std::string const& class_name, std::string const& instance_name = "");

      /// Create and add an algorithm to the sequence.
      ///
      /// **Example**
      /// @code
      /// Add<iguana::MyAlgorithm>("my_algorithm_name");
      /// @endcode
      /// @param instance_name a user-specified unique name for this algorithm instance;
      ///        if not specified, the class name will be used
      template <class ALGORITHM>
      void Add(std::string_view instance_name = "")
      {
        if(instance_name == "")
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
      ALGORITHM* Get(std::string const& instance_name)
      {
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
      template <typename OPTION_TYPE>
      void SetOption(std::string const& algo_name, std::string const& key, const OPTION_TYPE val)
      {
        Get<Algorithm>(algo_name)->SetOption(key, val);
      }

      /// Set the name of this sequence
      /// @param name the new name
      void SetName(std::string_view name);

      /// Print the names of the algorithms in this sequence
      /// @param level the log level of the printout
      void PrintSequence(Logger::Level level = Logger::info) const;

      /// @brief Set a custom configuration file for each algorithm in the sequence
      ///
      /// Use this function if you have a single configuration file for all the
      /// algorithms in your sequence
      /// @param name the configuration file name
      void SetConfigFileForEachAlgorithm(std::string const& name);

      /// @brief Set a custom configuration file directory for each algorithm in the sequence
      ///
      /// Use this function if you have a single configuration file directory for all the
      /// algorithms in your sequence
      /// @param name the directory name
      void SetConfigDirectoryForEachAlgorithm(std::string const& name);

      /// @brief Call a function for each algorithm in the sequence
      ///
      /// Use as:
      /// ```cpp
      /// ForEachAlgorithm([](auto& algo){ algo->...; });
      /// ```
      /// @param func the function to call for each algorithm `algo`
      void ForEachAlgorithm(std::function<void(algo_t&)> func);

    private:

      /// The sequence of algorithms
      std::vector<algo_t> m_sequence;

      /// Association of algorithm name to its index in the sequence
      std::unordered_map<std::string, std::vector<algo_t>::size_type> m_algo_names;
  };
}
