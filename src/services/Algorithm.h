#pragma once

#include "Logger.h"
#include <hipo4/bank.h>

namespace iguana {

  class Algorithm {

    public:

      using BankMap = std::unordered_map<std::string, hipo::bank>;

      Algorithm(std::string name);

      virtual void Start() = 0;
      virtual BankMap Run(BankMap inputBanks) = 0;
      virtual void Stop() = 0;
      virtual ~Algorithm() {}

    protected:
      std::shared_ptr<Logger> m_log;
  };
}
