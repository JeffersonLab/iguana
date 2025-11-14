#pragma once

#include "Algorithm.h"

namespace iguana {

  /// @algo_brief{An algorithm that can run a sequence of algorithms}
  ///
  /// This algorithm requires the use of `hipo::banklist`; there are neither `Run` functions which take
  /// individual `hipo::bank` parameters nor action functions. If you do not use `hipo::banklist`, you
  /// should use individual algorithms instead of this sequencing algorithm.
  ///
  /// Use the `Add` function to add algorithms to the sequence; the order is important, since
  /// the `Start`, `Run`, and `Stop` methods will sequentially call the corresponding algorithms' methods,
  /// in the same order that the algorithms were added to the sequence by `Add`.
  ///
  /// If an algorithm's `Run` function returns `false`, _i.e._, its "event-level" filter returns `false`, then `AlgorithmSequence`'s
  /// `Run` function will stop immediately and return `false`.
  ///
  /// @par Custom Event Filters
  /// If an algorithm's event-level filter is not adequate for your needs, and you want to tighten or override
  /// an algorithm's event-level filter, _i.e._, you want more control over how that algorithm's `Run` function return
  /// value is used, we recommond defining _two_ `AlgorithmSequence` instances. For example, suppose you want a tighter
  /// event-level filter from or after `algo2` in the sequence `algo1`, `algo2`, `algo3`, `algo4`; you may implement this by
  /// using _two_ sequences, where the first ends at `algo2`:
  /// @code
  /// // define sequences
  /// iguana::AlgorithmSequence seq1
  /// seq1.Add("algo1");
  /// seq1.Add("algo2");
  /// iguana::AlgorithmSequence seq2
  /// seq2.Add("algo3");
  /// seq2.Add("algo4");
  /// // start them
  /// seq1.Start(banks);
  /// seq2.Start(banks);
  /// @endcode
  /// Then, in your event loop, call your tighter filter between the sequences' `Run` calls:
  /// @code
  /// if(!seq1.Run(banks)) continue;
  /// if( /*your event filter */) continue;
  /// if(!seq2.Run(banks)) continue;
  /// @endcode
  ///
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
      /// @param algo_class_name the name of the algorithm class
      /// @param algo_instance_name a user-specified unique name for this algorithm instance;
      ///        if not specified, `algo_class_name` will be used
      void Add(std::string const& algo_class_name, std::string const& algo_instance_name = "");

      /// Create and add an algorithm to the sequence.
      ///
      /// **Example**
      /// @code
      /// Add<iguana::MyAlgorithm>("my_algorithm_name");
      /// @endcode
      /// @param algo_instance_name a user-specified unique name for this algorithm instance;
      ///        if not specified, the class name will be used
      template <class ALGORITHM>
      void Add(std::string_view algo_instance_name = "")
      {
        if(algo_instance_name == "")
          Add(std::make_unique<ALGORITHM>());
        else
          Add(std::make_unique<ALGORITHM>(algo_instance_name));
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
      /// @param algo_instance_name the instance name of the algorithm
      /// @return a reference to the algorithm
      template <class ALGORITHM = Algorithm>
      ALGORITHM* Get(std::string const& algo_instance_name)
      {
        if(auto it{m_algo_names.find(algo_instance_name)}; it != m_algo_names.end())
          return dynamic_cast<ALGORITHM*>(m_sequence[it->second].get());
        m_log->Error("cannot find algorithm '{}' in sequence", algo_instance_name);
        throw std::runtime_error("cannot Get algorithm");
      }

      /// Set an algorithm option
      /// @see `Algorithm::SetOption`
      /// @param algo_instance_name the algorithm instance name
      /// @param key the option name
      /// @param val the option value
      template <typename OPTION_TYPE>
      void SetOption(std::string const& algo_instance_name, std::string const& key, const OPTION_TYPE val)
      {
        Get<Algorithm>(algo_instance_name)->SetOption(key, val);
      }

      /// Set the name of this sequence
      /// @param name the new name
      void SetName(std::string_view name);

      /// Get the list of created bank names, for creator-type algorithms
      /// @see `AlgorithmSequence::GetCreatedBankName` for algorithms which create only one bank
      /// @param algo_instance_name the algorithm instance name
      /// @returns the list of new bank names
      std::vector<std::string> GetCreatedBankNames(std::string const& algo_instance_name) const noexcept(false);

      /// Get the created bank name, for creator-type algorithms which create only one new bank
      /// @see `AlgorithmSequence::GetCreatedBankNames` for algorithms which create more than one new bank
      /// @param algo_instance_name the algorithm instance name
      /// @returns the new bank name
      std::string GetCreatedBankName(std::string const& algo_instance_name) const noexcept(false);

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

      /// Get the index of a bank in a `hipo::banklist`; throws an exception if the bank is not found
      /// @param banks the list of banks this algorithm will use
      /// @param bank_name the name of the bank
      /// @param algo_instance_name the algorithm instance name,
      /// to disambiguate the case where two algorithms create a bank with the same name (_cf._ `variant` parameter of tools::GetBankIndex)
      /// @returns the `hipo::banklist` index of the bank
      /// @see tools::GetBankIndex for a function that is independent of algorithm
      /// @see GetCreatedBankIndex, a convenience method for _Iguana-created_ banks
      hipo::banklist::size_type GetBankIndex(
          hipo::banklist& banks,
          std::string const& bank_name,
          std::string const& algo_instance_name) const noexcept(false);

      /// Get the index of an _Iguana-created_ bank in a `hipo::banklist`; throws an exception if the bank is not found, or if the algorithm
      /// creates more than one bank
      /// @param banks the list of banks this algorithm will use
      /// @param algo_instance_name the algorithm instance name,
      /// to disambiguate the case where two algorithms create a bank with the same name (_cf._ `variant` parameter of tools::GetBankIndex)
      /// @returns the `hipo::banklist` index of the bank
      /// @see GetBankIndex for a more general method
      hipo::banklist::size_type GetCreatedBankIndex(
          hipo::banklist& banks,
          std::string const& algo_instance_name) const noexcept(false);

    private:

      /// The sequence of algorithms
      std::vector<algo_t> m_sequence;

      /// Association of algorithm name to its index in the sequence
      std::unordered_map<std::string, std::vector<algo_t>::size_type> m_algo_names;
  };
}
