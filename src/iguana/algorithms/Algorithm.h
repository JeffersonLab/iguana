#pragma once

#include <algorithm>
#include <functional>
#include <optional>
#include <set>
#include <unordered_map>
#include <variant>
#include <vector>

#include <hipo4/bank.h>

#include "iguana/algorithms/AlgorithmBoilerplate.h"
#include "iguana/services/Object.h"
#include "iguana/services/YAMLReader.h"

namespace iguana {

  /// Option value variant type
  /* NOTE: if you modify this, you also must modify:
   * - [ ] `PrintOptionValue`
   * - [ ] Template specializations in this class
   * - [ ] Template specializations in `YAMLReader` or `ConfigFileReader`
   * - [ ] Add new tests, if you added new types
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
      Algorithm(const std::string name)
          : Object(name)
          , m_rows_only(false)
          , m_default_config_file("")
          , o_user_config_file("")
          , o_user_config_dir("")
      {}
      virtual ~Algorithm() {}

      /// Initialize an algorithm before any events are processed, with the intent to process _banks_;
      /// use this method if you intend to use `Algorithm::Run`.
      /// @param banks the list of banks this algorithm will use, so that `Algorithm::Run` can cache the indices
      ///        of the banks that it needs
      virtual void Start(hipo::banklist& banks) = 0;

      /// Initialize an algorithm before any events are processed, with the intent to process _bank rows_ rather than full banks;
      /// use this method if you intend to use "action functions" instead of `Algorithm::Run`.
      void Start();

      /// Run an algorithm for an event
      /// @param banks the list of banks to process
      virtual void Run(hipo::banklist& banks) const = 0;

      /// Finalize an algorithm after all events are processed
      virtual void Stop() = 0;

      /// Set an option specified by the user. If the option name is `"log"`, the log level of the `Logger`
      /// owned by this algorithm will be changed to the specified value.
      /// @param key the name of the option
      /// @param val the value to set
      /// @returns the value that has been set (if needed, _e.g._, when `val` is an rvalue)
      template <typename OPTION_TYPE>
      OPTION_TYPE SetOption(const std::string key, const OPTION_TYPE val);

      /// Get the value of a scalar option
      /// @param key the unique key name of this option, for caching; if empty, the option will not be cached
      /// @param node_path the `YAML::Node` identifier path to search for this option in the config files; if empty, it will just use `key`
      /// @returns the scalar option
      template <typename OPTION_TYPE>
      OPTION_TYPE GetOptionScalar(const std::string key, YAMLReader::node_path_t node_path = {});

      /// Get the value of a vector option
      /// @param key the unique key name of this option, for caching; if empty, the option will not be cached
      /// @param node_path the `YAML::Node` identifier path to search for this option in the config files; if empty, it will just use `key`
      /// @returns the vector option
      template <typename OPTION_TYPE>
      std::vector<OPTION_TYPE> GetOptionVector(const std::string key, YAMLReader::node_path_t node_path = {});

      /// Get the value of a vector option, and convert it to `std::set`
      /// @param key the unique key name of this option
      /// @param node_path the `YAML::Node` identifier path to search for this option in the config files; if empty, it will just use `key`
      /// @returns the vector option converted to `std::set`
      template <typename OPTION_TYPE>
      std::set<OPTION_TYPE> GetOptionSet(const std::string key, YAMLReader::node_path_t node_path = {});

      /// Set the name of this algorithm
      /// @param name the new name
      void SetName(const std::string name);

      /// Get a reference to this algorithm's configuration (`YAMLReader`)
      /// @returns the configuration
      std::unique_ptr<YAMLReader>& GetConfig();

      /// Set a custom `YAMLReader` to use for this algorithm
      /// @param yaml_config the custom `YAMLReader` instance
      void SetConfig(std::unique_ptr<YAMLReader>&& yaml_config);

      /// Set a custom configuration file for this algorithm; see also `Algorithm::SetConfigDirectory`
      /// @param name the configuration file name
      void SetConfigFile(std::string name);

      /// Set a custom configuration file directory for this algorithm; see also `Algorithm::SetConfigFile`
      /// @param name the directory name
      void SetConfigDirectory(std::string name);

    protected: // methods

      /// Parse YAML configuration files. Sets `m_yaml_config`.
      void ParseYAMLConfig();

      /// Get the index of a bank in a `hipo::banklist`; throws an exception if the bank is not found
      /// @param banks the list of banks this algorithm will use
      /// @param bankName the name of the bank
      /// returns the `hipo::banklist` index of the bank
      hipo::banklist::size_type GetBankIndex(hipo::banklist& banks, const std::string bankName) const noexcept(false);

      /// Return a string with the value of an option along with its type
      /// @param key the name of the option
      /// @return the string value and its type
      std::string PrintOptionValue(const std::string key) const;

      /// Get the reference to a bank from a `hipo::banklist`; optionally checks if the bank name matches the expectation
      /// @param banks the `hipo::banklist` from which to get the specified bank
      /// @param idx the index of `banks` of the specified bank
      /// @param expectedBankName if specified, checks that the specified bank has this name
      /// @return a reference to the bank
      hipo::bank& GetBank(hipo::banklist& banks, const hipo::banklist::size_type idx, const std::string expectedBankName = "") const noexcept(false);

      /// Mask a row, setting all items to zero
      /// @param bank the bank to modify
      /// @param row the row to mask
      void MaskRow(hipo::bank& bank, const int row) const;

      /// Dump all banks in a `hipo::banklist`
      /// @param banks the banks to show
      /// @param message if specified, print a header message
      /// @param level the log level
      void ShowBanks(hipo::banklist& banks, const std::string message = "", const Logger::Level level = Logger::trace) const;

      /// Dump a single bank
      /// @param bank the bank to show
      /// @param message if specified, print a header message
      /// @param level the log level
      void ShowBank(hipo::bank& bank, const std::string message = "", const Logger::Level level = Logger::trace) const;

      /// Get an option from the option cache
      /// @param key the key name associated with this option
      /// @returns the option value, if found (using `std::optional`)
      template <typename OPTION_TYPE>
      std::optional<OPTION_TYPE> GetCachedOption(const std::string key) const;

    private: // methods

      /// Prepend `node_path` with the full algorithm name. If `node_path` is empty, set it to `{key}`.
      /// @param key the key name for this option
      /// @param node_path the `YAMLReader::node_path_t` to prepend
      void CompleteOptionNodePath(const std::string key, YAMLReader::node_path_t& node_path) const;

    protected: // members

      /// Class name of this algorithm
      std::string m_class_name;

      /// Data structure to hold configuration options
      std::unordered_map<std::string, option_t> m_option_cache;

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

    private: // members

      /// YAML reader
      std::unique_ptr<YAMLReader> m_yaml_config;
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
      static bool Register(const std::string& name, algo_creator_t creator) noexcept;

      /// Create an algorithm.
      /// @param name the name of the algorithm, which was used as an argument in the `AlgorithmFactory::Register` call
      static algo_t Create(const std::string& name) noexcept;

    private:

      /// Association between the algorithm names and their creators
      static std::unordered_map<std::string, algo_creator_t> s_creators;
  };
}
