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

      /// Read a value from the opened INI file which is at a given section and key.
      /// This function can take in any C++ type.
      /// @param section the value's section in the INI file.
      /// @param key the value's key in the section in the INI file.
      /// @param defaultValue the function will default to this if the section or key do not exist.
      /// @returns the value at the section and key in the INI file.
      template <typename T> T readValue(const std::string &section, const std::string &key, T defaultValue);

      /// Read an array from the opened INI file which is at a given section and key.
      /// This function can take in any C++ type.
      /// @param section the array's section in the INI file.
      /// @param key the array's key in the section in the INI file.
      /// @param defaultValue the function will default to this if the section or key do not exist.
      /// @returns the array at the section and key in the INI file returned as a std::vector.
      template <typename T> std::vector<T> readArray(const std::string &section, const std::string &key, const std::vector<T> &defaultValue);

    protected:

      /// The file to be opened by INIReader
      std::string m_file;

      /// GKeyFile used to open file
      GKeyFile *m_keyfile;
  };
}
