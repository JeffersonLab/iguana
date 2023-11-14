#pragma once

#include "Logger.h"
#include <hipo4/bank.h>
#include <set>

namespace iguana {

  class Algorithm {

    public:

      using BankMap = std::unordered_map<std::string, hipo::bank>;

      /// Algorithm base class constructor
      /// @param name the unique name for a derived class instance
      Algorithm(std::string name);

      /// Algorithm base class destructor
      virtual ~Algorithm() {}

      /// Initialize an algorithm before any events are processed
      virtual void Start() = 0;

      /// Run an algorithm
      /// @param inputBanks the set of input banks
      /// @return a set of output banks
      virtual BankMap Run(BankMap inputBanks) = 0;

      /// Finalize an algorithm after all events are processed
      virtual void Stop() = 0;

    protected:

      /// Check if `banks` contains all keys `keys`; this is useful for checking algorithm inputs are complete.
      /// @param banks the set of (key,bank) pairs to check
      /// @keys the required keys
      /// @return true if `banks` is missing any keys in `keys`
      bool MissingInputBanks(BankMap banks, std::set<std::string> keys);

      /// Throw a runtime exception when calling `Run`
      void ThrowRun();

      /// algorithm name
      std::string m_name;

      /// Logger
      std::shared_ptr<Logger> m_log;
  };
}
