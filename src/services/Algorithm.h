#pragma once

#include "Logger.h"
#include <hipo4/bank.h>
#include <set>
#include <vector>

namespace iguana {

  class Algorithm {

    public:

      using BankVec = std::vector<std::shared_ptr<hipo::bank>>;

      /// Algorithm base class constructor
      /// @param name the unique name for a derived class instance
      Algorithm(std::string name);

      /// Algorithm base class destructor
      virtual ~Algorithm() {}

      /// Initialize an algorithm before any events are processed.
      /// The `Run` method will assume a default ordering of banks. 
      /// Derived classes likely do not need to override this method.
      virtual void Start();

      /// Initialize an algorithm before any events are processed
      /// @param bankVecOrder The `Run` method will use this ordering
      virtual void Start(std::unordered_map<std::string, int> bankVecOrder) = 0;

      /// Run an algorithm
      /// @param inBanks the set of banks to process
      virtual void Run(BankVec inBanks) = 0;

      /// Finalize an algorithm after all events are processed
      virtual void Stop() = 0;

    protected:

      /// Copy a row from one bank to another, assuming their schemata are equivalent
      /// @param srcBank the source bank
      /// @param srcRow the row in `srcBank` to copy from
      /// @param destBank the destination bank
      /// @param destRow the row in `destBank` to copy to
      void CopyBankRow(std::shared_ptr<hipo::bank> srcBank, int srcRow, std::shared_ptr<hipo::bank> destBank, int destRow);

      /// Blank a row, setting all items to zero
      /// @param bank the bank to modify
      /// @param row the row to blank
      void BlankRow(std::shared_ptr<hipo::bank> bank, int row);

      /// Dump all banks in a BankVec
      /// @param banks the banks to show
      /// @param message optionally print a header message
      /// @param level the log level
      void ShowBanks(BankVec banks, std::string message="", Logger::Level level=Logger::trace);

      /// Dump a single bank
      /// @param bank the bank to show
      /// @param message optionally print a header message
      /// @param level the log level
      void ShowBank(std::shared_ptr<hipo::bank> bank, std::string message="", Logger::Level level=Logger::trace);

      /// Stop the algorithm and throw a runtime exception
      /// @param message the error message
      void Throw(std::string message);

      /// algorithm name
      std::string m_name;

      /// list of required banks
      std::set<std::string> m_requiredBanks;

      /// Logger
      std::shared_ptr<Logger> m_log;
  };
}
