#pragma once

#include "Logger.h"
#include <hipo4/bank.h>
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
      /// @param bankVecIndices The `Run` method will use these indices to access banks
      virtual void Start(std::unordered_map<std::string, int> bankVecIndices) = 0;

      /// Run an algorithm
      /// @param banks the set of banks to process
      virtual void Run(BankVec banks) = 0;

      /// Finalize an algorithm after all events are processed
      virtual void Stop() = 0;

    protected:

      /// Cache the index of a bank in a `BankVec`; throws an exception if the bank is not found
      /// @param bankVecIndices the relation between bank name and `BankVec` index
      /// @param idx a reference to the `BankVec` index of the bank
      /// @param bankName the name of the bank
      void CacheBankIndex(std::unordered_map<std::string, int> bankVecIndices, int &idx, std::string bankName);

      /// Get the pointer to a bank from a `BankVec`; optionally checks if the bank name matches the expectation
      /// @param banks the `BankVec` from which to get the specified bank
      /// @param idx the index of `banks` of the specified bank
      /// @param expectedBankName if specified, checks that the specified bank has this name
      std::shared_ptr<hipo::bank> GetBank(BankVec banks, int idx, std::string expectedBankName="");

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
      std::vector<std::string> m_requiredBanks;

      /// Logger
      std::shared_ptr<Logger> m_log;
  };
}
