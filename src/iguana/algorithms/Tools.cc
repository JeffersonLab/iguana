#include "Tools.h"

namespace iguana::tools {

  hipo::banklist::size_type GetBankIndex(
      hipo::banklist& banks,
      std::string const& bank_name,
      unsigned int const& variant) noexcept(false)
  {
    unsigned int num_found = 0;
    for(hipo::banklist::size_type i = 0; i < banks.size(); i++) {
      auto& bank = banks.at(i);
      if(bank.getSchema().getName() == bank_name) {
        if(num_found == variant)
          return i;
        num_found++;
      }
    }
    throw std::runtime_error("GetBankIndex failed to find bank \"" + bank_name + "\"");
  }

}
