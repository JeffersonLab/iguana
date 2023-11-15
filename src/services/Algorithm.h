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
      /// @param inBanks the set of input banks
      /// @return a set of output banks
      virtual BankMap Run(BankMap inBanks) = 0;

      /// Finalize an algorithm after all events are processed
      virtual void Stop() = 0;

    protected:

      /// Check if `banks` contains all keys `keys`; this is useful for checking algorithm inputs are complete.
      /// @param banks the set of (key,bank) pairs to check
      /// @keys the required keys
      /// @return true if `banks` is missing any keys in `keys`
      bool MissingInputBanks(BankMap banks, std::set<std::string> keys);

      /// Copy a row from one bank to another, assuming their schemata are equivalent
      /// @param srcBank the source bank
      /// @param destBank the destination bank
      /// @param row the row to copy from `srcBank` to `destBank`
      void CopyBankRow(hipo::bank srcBank, hipo::bank destBank, int row);

      /// Blank a row, setting all items to zero
      /// @param bank the bank to modify
      /// @param row the row to blank
      void BlankRow(hipo::bank bank, int row);

      /// Throw a runtime exception when calling `Run`
      void ThrowRun();

      /// Dump all banks in a BankMap
      /// @param banks the banks to show
      /// @param message optionally print a header message
      /// @param level the log level
      void ShowBanks(BankMap banks, std::string message="", Logger::Level level=Logger::trace);

      /// Dump all input and output banks
      /// @param inBanks the input banks
      /// @param outBanks the output banks
      /// @param level the log level
      void ShowBanks(BankMap inBanks, BankMap outBanks, Logger::Level level=Logger::trace);

      /// algorithm name
      std::string m_name;

      /// Logger
      std::shared_ptr<Logger> m_log;
  };
}
