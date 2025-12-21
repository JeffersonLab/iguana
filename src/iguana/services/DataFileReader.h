#pragma once

#include "ConfigFileReader.h"

namespace iguana {

  /// @brief A data file reader, for example, weights files for machine learning-dependent algorithms
  class DataFileReader : public ConfigFileReader
  {

    public:

      /// @param datadir_subdir the subdirectory within build option `datadir` where the file may be found
      /// @param name of this reader (for `Logger`)
      DataFileReader(std::string_view datadir_subdir = "", std::string_view name = "data_file");
      ~DataFileReader() {}
  };
}
