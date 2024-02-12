#pragma once

#include <variant>
#include <vector>
#include <set>
#include <unordered_map>
#include <algorithm>
#include <functional>

#include <hipo4/bank.h>

#include "iguana/services/Object.h"
#include "iguana/services/YAMLReader.h"
#include "iguana/algorithms/AlgorithmBoilerplate.h"

namespace iguana {

  /// Option value variant type
  using option_t = std::variant<
    int,
    double,
    std::string,
    std::vector<int>,
    std::vector<double>
  >;

  /// @brief Base class for all algorithms to inherit from
  ///
  /// This is the base class for all algorithms. It provides common members, such as
  /// a logger instance and options data structure. Algorithm implementations must:
  /// - inherit from this base class
  /// - override the methods `Algorithm::Start`, `Algorithm::Run` and `Algorithm::Stop`
  ///
  /// See existing algorithms for examples.
  class Algorithm : public Object {

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
      template <typename OPTION_TYPE>
        void SetOption(const std::string key, const OPTION_TYPE val) {
          if(key == "log") {
            if constexpr(std::disjunction<
                std::is_same<OPTION_TYPE, std::string>,
                std::is_same<OPTION_TYPE, const char*>,
                std::is_same<OPTION_TYPE, Logger::Level>
                >::value
                )
              m_log->SetLevel(val);
            else
              m_log->Error("Option '{}' must be a string or a Logger::Level", key);
          }
          else {
            m_opt[key] = val;
            m_log->Debug("User set option '{}' = {}", key, PrintOptionValue(key));
          }
        }

      /// Set the name of this algorithm
      /// @param name the new name
      void SetName(const std::string name);

      /// Set a custom `YAMLReader` to use for this algorithm
      /// @param yaml_config the custom `YAMLReader` instance
      void SetYAMLConfig(std::unique_ptr<YAMLReader>&& yaml_config);

    protected: // methods

      /// Parse YAML configuration files. Sets `m_yaml_config`.
      void ParseYAMLConfig();

      /// Cache the index of a bank in a `hipo::banklist`; throws an exception if the bank is not found
      /// @param[in] banks the list of banks this algorithm will use
      /// @param[in] bankName the name of the bank
      /// @param[out] idx a reference to the `hipo::banklist` index of the bank
      void CacheBankIndex(hipo::banklist& banks, const std::string bankName, hipo::banklist::size_type& idx) const noexcept(false);

      /// Cache an option specified by the user, and define its default value. If the user-specified
      /// option has the wrong type, an error will be printed and the default value will be used instead.
      /// @param[in] key the name of the option
      /// @param[in] def the default value
      /// @param[out] val reference to the value of the option, to be cached by `Algorithm::Start`
      template <typename OPTION_TYPE>
        void CacheOption(const std::string key, const OPTION_TYPE def, OPTION_TYPE& val) {
          bool get_error = false;
          if(auto it{m_opt.find(key)}; it != m_opt.end()) { // cache the user's option value
            try { // get the expected type
              val = std::get<OPTION_TYPE>(it->second);
            } catch(const std::bad_variant_access& ex1) {
              m_log->Error("user option '{}' set to '{}', which is the wrong type...", key, PrintOptionValue(key));
              get_error = true;
              val = def;
            }
          }
          else { // cache the default option value
            val = def;
          }
          // sync `m_opt` to match the cached value `val` (e.g., so we can use `PrintOptionValue` to print it)
          m_opt[key] = val;
          if(get_error)
            m_log->Error("...using default value '{}' instead", PrintOptionValue(key));
          m_log->Debug("OPTION: {:>20} = {}", key, PrintOptionValue(key));
        }

      /// Cache an option of type `std::vector` and convert it to `std::set`
      /// @see `Algorithm::CacheOption`
      /// @param[in] key the name of the option
      /// @param[in] def the default `std::vector`
      /// @param[out] val reference to the `std::set` option
      template <typename T>
        void CacheOptionToSet(const std::string key, const std::vector<T> def, std::set<T>& val) {
          std::vector<T> vec;
          CacheOption(key, def, vec);
          std::copy(vec.begin(), vec.end(), std::inserter(val, val.end()));
        }

      /// Return a string with the value of an option along with its type
      /// @param key the name of the option
      /// @return the string value and its type
      std::string PrintOptionValue(const std::string key) const;

      /// Get the reference to a bank from a `hipo::banklist`; optionally checks if the bank name matches the expectation
      /// @param banks the `hipo::banklist` from which to get the specified bank
      /// @param idx the index of `banks` of the specified bank
      /// @param expectedBankName if specified, checks that the specified bank has this name
      /// @return a reference to the bank
      hipo::bank& GetBank(hipo::banklist& banks, const hipo::banklist::size_type idx, const std::string expectedBankName="") const noexcept(false);

      /// Mask a row, setting all items to zero
      /// @param bank the bank to modify
      /// @param row the row to mask
      void MaskRow(hipo::bank& bank, const int row) const;

      /// Dump all banks in a `hipo::banklist`
      /// @param banks the banks to show
      /// @param message if specified, print a header message
      /// @param level the log level
      void ShowBanks(hipo::banklist& banks, const std::string message="", const Logger::Level level=Logger::trace) const;

      /// Dump a single bank
      /// @param bank the bank to show
      /// @param message if specified, print a header message
      /// @param level the log level
      void ShowBank(hipo::bank& bank, const std::string message="", const Logger::Level level=Logger::trace) const;

    protected: // members

      /// Data structure to hold configuration options
      std::unordered_map<std::string, option_t> m_opt;

      /// If true, algorithm can only operate on bank _rows_; `Algorithm::GetBank`, and therefore `Algorithm::Run`, cannot be called
      bool m_rows_only;

      /// Default configuration file name
      std::string m_default_config_file;

      /// User's configuration file name, which may override the default configuration file, `m_default_config_file`.
      /// Set with `Algorithm::SetOption` using key `"config_file"`.
      std::string o_user_config_file;

      /// User's configuration file directory.
      /// Set with `Algorithm::SetOption` using key `"config_dir"`.
      std::string o_user_config_dir;

      /// YAML reader
      std::unique_ptr<YAMLReader> m_yaml_config;

    private:

  };

  //////////////////////////////////////////////////////////////////////////////

  /// Algorithm pointer type
  using algo_t = std::unique_ptr<Algorithm>;

  /// @brief Factory to create an algorithm.
  class AlgorithmFactory {

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
