#pragma once

#include <vector>
#include <string>
#include <memory>
#include <sstream>

#include <glib.h>

namespace iguana {

  /// @brief An INIReader based on GKey
  class INIReader {

    public:

      /// @param file the file to be opened by INIReader
      INIReader(const std::string file="");
      ~INIReader() {g_key_file_free(m_keyfile);}

      /// Get the file opened by INIReader
      std::string GetFile() const;

      template <typename T> T readValue(const std::string &section, const std::string &key, T defaultValue);

      template <typename T> std::vector<T> readArray(const std::string &section, const std::string &key, const std::vector<T> &defaultValue);

    protected:

      /// The file to be opened by INIReader
      std::string m_file;

      // Create a GKeyFile
      GKeyFile *m_keyfile;
  };
}
