#include <vector>
#include <string>

namespace iguana {

  struct BankColDef {
    std::string name;
    std::string type;
  };

  struct BankDef {
    std::string name;
    int group;
    int item;
    std::vector<BankColDef> entries;
  };

  static std::vector<BankDef> const bank_defs;

}
