#include <vector>
#include <string>


// FIXME
// This really ought to be a YAML file, however:
// - could be handled by `ConfigFileReader`, but need to prevent users from accidentally overriding this
//   file with `IGUANA_CONFIG_PATH`
// - we could generate this header file, given a YAML file, with Python (not Ruby, since Meson already needs Python, but Ruby
//   is only needed for Chameleon)

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

  std::vector<BankDef> const bank_defs{
    {
      .name = "physics::InclusiveKinematics",
        .group = 30000,
        .item = 1,
        .entries = {
          { .name = "pindex", .type = "S" },
          { .name = "Q2", .type = "D"},
          { .name = "x", .type = "D"},
          { .name = "y", .type = "D"},
          { .name = "W", .type = "D"},
          { .name = "nu", .type = "D"},
          { .name = "qx", .type = "D"},
          { .name = "qy", .type = "D"},
          { .name = "qz", .type = "D"},
          { .name = "qE", .type = "D"},
          { .name = "beamPz", .type = "D"},
          { .name = "targetM", .type = "D"}
        }
    }
  };

}

