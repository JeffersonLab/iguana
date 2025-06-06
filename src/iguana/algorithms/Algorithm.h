#pragma once

#include <mutex>
#include <optional>
#include <set>

#include <hipo4/bank.h>

#include "AlgorithmBoilerplate.h"
#include "iguana/bankdefs/BankDefs.h"
#include "iguana/services/YAMLReader.h"
#include <iguana/services/GlobalParam.h>

namespace iguana {

  /// Option value variant type
  /* NOTE: if you modify this, you also must modify:
   * - [ ] `PrintOptionValue`
   * - [ ] Template specializations in this class
   * - [ ] Template specializations in `YAMLReader` or `ConfigFileReader`, and `ConcurrentParam`
   * - [ ] Add new tests, if you added new types
   * - FIXME: adding `bool` type may be tricky, see https://github.com/JeffersonLab/iguana/issues/347
   */
  using option_t = std::variant<
      int,
      double,
      std::string,
      std::vector<int>,
      std::vector<double>,
      std::vector<std::string>>;

  /// @brief Base class for all algorithms to inherit from
  ///
  /// This is the base class for all algorithms. It provides common members, such as
  /// a logger instance and options data structure. Algorithm implementations must:
  /// - inherit from this base class
  /// - override the methods `Algorithm::Start`, `Algorithm::Run` and `Algorithm::Stop`
  ///
  /// See existing algorithms for examples.
  class Algorithm : public Object
  {

    public:

      /// @param name the unique name for a derived class instance
      Algorithm(std::string_view name)
          : Object(name)
          , m_rows_only(false)
          , m_default_config_file("")
          , o_user_config_file("")
          , o_user_config_dir("")
      {}
      virtual ~Algorithm() {}

      /// @brief Initialize this algorithm before any events are processed, with the intent to process _banks_
      ///
      /// use this method if you intend to use `Algorithm::Run`.
      /// @param banks the list of banks this algorithm will use, so that `Algorithm::Run` can cache the indices
      ///        of the banks that it needs
      virtual void Start(hipo::banklist& banks) = 0;

      /// @brief Initialize this algorithm before any events are processed, with the intent to process _bank rows_ rather than full banks;
      ///
      /// use this method if you intend to use "action functions" instead of `Algorithm::Run`.
      void Start();

      /// @brief Run this algorithm for an event.
      /// @param banks the list of banks to process
      virtual void Run(hipo::banklist& banks) const = 0;

      /// @brief Finalize this algorithm after all events are processed.
      virtual void Stop() = 0;

      /// Set an option specified by the user. If the option name is `"log"`, the log level of the `Logger`
      /// owned by this algorithm will be changed to the specified value.
      /// @param key the name of the option
      /// @param val the value to set
      /// @returns the value that has been set (if needed, _e.g._, when `val` is an rvalue)
      template <typename OPTION_TYPE>
      OPTION_TYPE SetOption(std::string const& key, const OPTION_TYPE val)
      {
        // FIXME: this template is not specialized, to be friendlier to python `cppyy` bindings
        if(key == "log") {
          if constexpr(std::disjunction<
                           std::is_same<OPTION_TYPE, std::string>,
                           std::is_same<OPTION_TYPE, char const*>,
                           std::is_same<OPTION_TYPE, Logger::Level>>::value)
            m_log->SetLevel(val);
          else
            m_log->Error("Option '{}' must be a string or a Logger::Level", key);
        }
        m_option_cache[key] = val;
        return val;
      }

      /// Get the value of a scalar option
      /// @param key the unique key name of this option, for caching; if empty, the option will not be cached
      /// @param node_path the `YAML::Node` identifier path to search for this option in the config files; if empty, it will just use `key`
      /// @returns the scalar option
      template <typename OPTION_TYPE>
      OPTION_TYPE GetOptionScalar(std::string const& key, YAMLReader::node_path_t node_path = {}) const;

      /// Get the value of a vector option
      /// @param key the unique key name of this option, for caching; if empty, the option will not be cached
      /// @param node_path the `YAML::Node` identifier path to search for this option in the config files; if empty, it will just use `key`
      /// @returns the vector option
      template <typename OPTION_TYPE>
      std::vector<OPTION_TYPE> GetOptionVector(std::string const& key, YAMLReader::node_path_t node_path = {}) const;

      /// Get the value of a vector option, and convert it to `std::set`
      /// @param key the unique key name of this option
      /// @param node_path the `YAML::Node` identifier path to search for this option in the config files; if empty, it will just use `key`
      /// @returns the vector option converted to `std::set`
      template <typename OPTION_TYPE>
      std::set<OPTION_TYPE> GetOptionSet(std::string const& key, YAMLReader::node_path_t node_path = {}) const;

      /// Set the name of this algorithm
      /// @param name the new name
      void SetName(std::string_view name);

      /// Get a reference to this algorithm's configuration (`YAMLReader`)
      /// @returns the configuration
      std::unique_ptr<YAMLReader> const& GetConfig() const;

      /// Set a custom `YAMLReader` to use for this algorithm
      /// @param yaml_config the custom `YAMLReader` instance
      void SetConfig(std::unique_ptr<YAMLReader>&& yaml_config);

      /// Set a custom configuration file for this algorithm
      /// @see `Algorithm::SetConfigDirectory`
      /// @param name the configuration file name
      void SetConfigFile(std::string const& name);

      /// Set a custom configuration file directory for this algorithm
      /// @see `Algorithm::SetConfigFile`
      /// @param name the directory name
      void SetConfigDirectory(std::string const& name);

    protected: // methods

      /// Parse YAML configuration files. Sets `m_yaml_config`.
      void ParseYAMLConfig();

      /// Get the reference to a bank from a `hipo::banklist`; optionally checks if the bank name matches the expectation
      /// @param banks the `hipo::banklist` from which to get the specified bank
      /// @param idx the index of `banks` of the specified bank
      /// @param expected_bank_name if specified, checks that the specified bank has this name
      /// @return a reference to the bank
      hipo::bank& GetBank(hipo::banklist& banks, hipo::banklist::size_type const idx, std::string const& expected_bank_name = "") const noexcept(false);

      /// Get the index of a bank in a `hipo::banklist`; throws an exception if the bank is not found
      /// @param banks the list of banks this algorithm will use
      /// @param bank_name the name of the bank
      /// returns the `hipo::banklist` index of the bank
      hipo::banklist::size_type GetBankIndex(hipo::banklist& banks, std::string const& bank_name) const noexcept(false);

      /// Create a new bank and push it to the bank list. The bank must be defined in `src/iguana/bankdefs/iguana.json`.
      /// @param [out] banks the `hipo::banklist` onto which the new bank will be pushed
      /// @param [out] bank_idx will be set to the `hipo::banklist` index of the new bank
      /// @param [in] bank_name the new bank name
      /// @returns the bank's schema
      hipo::schema CreateBank(
          hipo::banklist& banks,
          hipo::banklist::size_type& bank_idx,
          std::string const& bank_name) const noexcept(false);

      /// Dump all banks in a `hipo::banklist`
      /// @param banks the banks to show
      /// @param message if specified, print a header message
      /// @param level the log level
      void ShowBanks(hipo::banklist& banks, std::string_view message = "", Logger::Level const level = Logger::trace) const;

      /// Dump a single bank
      /// @param bank the bank to show
      /// @param message if specified, print a header message
      /// @param level the log level
      void ShowBank(hipo::bank& bank, std::string_view message = "", Logger::Level const level = Logger::trace) const;

      /// Get an option from the option cache
      /// @param key the key name associated with this option
      /// @returns the option value, if found (using `std::optional`)
      template <typename OPTION_TYPE>
      std::optional<OPTION_TYPE> GetCachedOption(std::string const& key) const;

    private: // methods

      /// Prepend `node_path` with the full algorithm name. If `node_path` is empty, set it to `{key}`.
      /// @param key the key name for this option
      /// @param node_path the `YAMLReader::node_path_t` to prepend
      void CompleteOptionNodePath(std::string const& key, YAMLReader::node_path_t& node_path) const;

      // PrintOptionValue: overloaded for different value types
      void PrintOptionValue(std::string const& key, int const& val, Logger::Level const level = Logger::debug, std::string_view prefix = "OPTION") const;
      void PrintOptionValue(std::string const& key, double const& val, Logger::Level const level = Logger::debug, std::string_view prefix = "OPTION") const;
      void PrintOptionValue(std::string const& key, std::string const& val, Logger::Level const level = Logger::debug, std::string_view prefix = "OPTION") const;
      void PrintOptionValue(std::string const& key, std::vector<int> const& val, Logger::Level const level = Logger::debug, std::string_view prefix = "OPTION") const;
      void PrintOptionValue(std::string const& key, std::vector<double> const& val, Logger::Level const level = Logger::debug, std::string_view prefix = "OPTION") const;
      void PrintOptionValue(std::string const& key, std::vector<std::string> const& val, Logger::Level const level = Logger::debug, std::string_view prefix = "OPTION") const;

    protected: // members

      /// Class name of this algorithm
      std::string m_class_name;

      /// If true, algorithm can only operate on bank _rows_; `Algorithm::GetBank`, and therefore `Algorithm::Run`, cannot be called
      bool m_rows_only;

      /// Default configuration file name
      std::string m_default_config_file;

      /// User's configuration file name, which may override the default configuration file, `m_default_config_file`.
      /// Set it with `Algorithm::SetConfigFile`
      std::string o_user_config_file;

      /// User's configuration file directory.
      /// Set it with `Algorithm::SetConfigDirectory`
      std::string o_user_config_dir;

      /// A mutex for this algorithm
      mutable std::mutex m_mutex;

    private: // members

      /// YAML reader
      std::unique_ptr<YAMLReader> m_yaml_config;

      /// Data structure to hold configuration options set by `Algorithm::SetOption`
      std::unordered_map<std::string, option_t> m_option_cache;

  };

  //////////////////////////////////////////////////////////////////////////////

  /// Algorithm pointer type
  using algo_t = std::unique_ptr<Algorithm>;

  /// @brief Factory to create an algorithm.
  class AlgorithmFactory
  {

    public:

      /// Algorithm creator function type
      using algo_creator_t = std::function<algo_t()>;

      AlgorithmFactory() = delete;

      /// Register an algorithm with a unique name. Algorithms register themselves by calling this function.
      /// @param name the name of the algorithm (not equivalent to `Object::m_name`)
      /// @param creator the creator function
      /// @param new_banks if this algorithm creates *new* banks, list them here
      /// @returns true if the algorithm has not yet been registered
      static bool Register(std::string const& name, algo_creator_t creator, std::vector<std::string> const new_banks = {}) noexcept;

      /// Create an algorithm. Throws an exception if the algorithm cannot be created
      /// @param name the name of the algorithm, which was used as an argument in the `AlgorithmFactory::Register` call
      /// @returns the algorithm instance
      static algo_t Create(std::string const& name) noexcept(false);

      /// Check if a bank is created by an algorithm
      /// @param bank_name the name of the bank
      /// @returns the list of algorithms which create it, if any
      static std::optional<std::vector<std::string>> QueryNewBank(std::string const& bank_name) noexcept;

    private:

      /// Association between the algorithm names and their creators
      static std::unordered_map<std::string, algo_creator_t> s_creators;

      /// Association between a created bank and its creator algorithms
      static std::unordered_map<std::string, std::vector<std::string>> s_created_banks;
  };
}
