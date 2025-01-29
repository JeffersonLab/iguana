#include "DataFileReader.h"

namespace iguana {
  DataFileReader::DataFileReader(std::string_view datadir_subdir, std::string_view name) : ConfigFileReader(name, false)
  {
    // first, add the "fallback" directory; this is the lowest priority directory, relying
    // on the environment variable '$IGUANA', in case the higher-priority, build-time path fails
    auto user_prefix = std::getenv("IGUANA");
    if(user_prefix != nullptr)
      AddDirectory(std::string(user_prefix) + "/" + std::string(IGUANA_DATADIR) + "/" + std::string(datadir_subdir));
    // then add the hard-coded path, which is set at build time; if Iguana installation is
    // relocated, this path will be wrong and the above fallback will be used instead
    AddDirectory(std::string(IGUANA_PREFIX) + "/" + std::string(IGUANA_DATADIR) + "/" + std::string(datadir_subdir));
  }
}
