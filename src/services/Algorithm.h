/// @file
/// @brief Base class for all algorithms to inherit from

#pragma once

#include "TypeDefs.h"
#include "Logger.h"

namespace iguana {

  /// This is the base class for all algorithms. It provides common members, such as
  /// a logger instance and options data structure. Algorithm implementations must:
  /// - inherit from this base class
  /// - override the methods `Algorithm::Start`, `Algorithm::Run` and `Algorithm::Stop`
  ///
  /// See existing algorithms for examples.
  class Algorithm {

    public:

      /// Algorithm base class constructor
      /// @param name the unique name for a derived class instance
      Algorithm(const std::string name);

      /// Algorithm base class destructor
      virtual ~Algorithm() {}

      /// Initialize an algorithm before any events are processed.
      virtual void Start();

      /// Initialize an algorithm before any events are processed
      /// @param index_cache The `Algorithm::Run` method will use these indices to access banks
      virtual void Start(const bank_index_cache_t& index_cache) = 0;

      /// Run an algorithm for an event
      /// @param banks the list of banks to process
      virtual void Run(hipo::banklist& banks) const = 0;

      /// Finalize an algorithm after all events are processed
      virtual void Stop() = 0;

      /// Set an option specified by the user
      /// @param key the name of the option
      /// @param val the value to set
      void SetOption(const std::string key, const option_value_t val);

      /// Get the logger
      /// @return the logger used by this algorithm
      std::unique_ptr<Logger>& Log();

    protected:

      /// Cache the index of a bank in a `hipo::banklist`; throws an exception if the bank is not found
      /// @param index_cache the relation between bank name and `hipo::banklist` index
      /// @param idx a reference to the `hipo::banklist` index of the bank
      /// @param bankName the name of the bank
      void CacheBankIndex(const bank_index_cache_t index_cache, int& idx, const std::string bankName) const noexcept(false);

      /// Cache an option specified by the user, and define its default value. If the user-specified
      /// option has the wrong type, an error will be printed and the default value will be used instead.
      /// @param key the name of the option
      /// @param def the default value
      /// @param val reference to the value of the option, to be cached by `Algorithm::Start`
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

      /// Return a string with the value of an option along with its type
      /// @param key the name of the option
      /// @return the string value and its type
      std::string PrintOptionValue(const std::string key) const;

      /// Get the reference to a bank from a `hipo::banklist`; optionally checks if the bank name matches the expectation
      /// @param banks the `hipo::banklist` from which to get the specified bank
      /// @param idx the index of `banks` of the specified bank
      /// @param expectedBankName if specified, checks that the specified bank has this name
      /// @return a reference to the bank
      hipo::bank& GetBank(hipo::banklist& banks, const int idx, const std::string expectedBankName="") const noexcept(false);

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

      /// Stop the algorithm and throw a runtime exception
      /// @param message the error message
      void Throw(const std::string message) const noexcept(false);

      /// algorithm name
      const std::string m_name;

      /// list of required banks
      std::vector<std::string> m_requiredBanks;

      /// `Logger` instance for this algorithm
      std::unique_ptr<Logger> m_log;

      /// Configuration options
      options_t m_opt;
  };
}
