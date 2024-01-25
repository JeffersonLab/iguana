#include "INIReader.h"

namespace iguana {

  INIReader::INIReader(const std::string file) :
    m_file(file)
  {
    m_keyfile = g_key_file_new();

    // Load the INI file
    GError *error = nullptr;
    if (!g_key_file_load_from_file(m_keyfile, file.c_str(), G_KEY_FILE_NONE, &error))
    {
      g_printerr("Error loading INI file: %s\n", error->message);
      g_error_free(error);
      g_key_file_free(m_keyfile);
    }
  }

  std::string INIReader::GetFile() const {
    return m_file;
  }

  // Template function to read values of any C++ type from GKeyFile
  template <typename T>
  T INIReader::readValue(const std::string &section, const std::string &key, T defaultValue)
  {
    GError *error = nullptr;

    // Get the raw string value from GKeyFile
    gchar *rawValue = g_key_file_get_string(m_keyfile, section.c_str(), key.c_str(), &error);

    if (error != nullptr)
    {
      g_error_free(error);
      return defaultValue;
    }

    // Convert the raw string value to the desired C++ type
    T value;
    std::istringstream(rawValue) >> value;

    // Free the raw string value
    g_free(rawValue);

    return value;
  }

  // Explicit instantiation for double
  template double INIReader::readValue<double>(const std::string &, const std::string &, const double);
  // Explicit instantiation for int
  template int INIReader::readValue<int>(const std::string &, const std::string &, const int);
  // Explicit instantiation for std::string
  template std::string INIReader::readValue<std::string>(const std::string &, const std::string &, const std::string);

  // Function to read an array of any C++ type from GKeyFile
  template <typename T>
  std::vector<T> INIReader::readArray(const std::string &section, const std::string &key, const std::vector<T> &defaultValue)
  {
    GError *error = nullptr;

    // Get the raw string value from GKeyFile
    gchar *rawValue = g_key_file_get_string(m_keyfile, section.c_str(), key.c_str(), &error);

    if (error != nullptr)
    {
      g_error_free(error);
      return defaultValue;
    }

    // Convert the raw string value to a vector of the desired C++ type
    std::istringstream iss(rawValue);
    std::vector<T> value;
    std::string element;

    // Read the entire line as a string
    std::getline(iss, element);

    // Tokenize the string using ',' as the delimiter
    std::istringstream elementStream(element);
    while (std::getline(elementStream, element, ','))
    {
      // Trim leading and trailing whitespaces
      element.erase(0, element.find_first_not_of(" \t"));
      element.erase(element.find_last_not_of(" \t") + 1);

      // Convert the element to the desired type
      T convertedElement;
      std::istringstream(element) >> convertedElement;

      value.push_back(convertedElement);
    }

    // Free the raw string value
    g_free(rawValue);

    return value;
  }

  // Explicit instantiation for double
  template std::vector<double> INIReader::readArray<double>(const std::string &, const std::string &, const std::vector<double> &);
  // Explicit instantiation for int
  template std::vector<int> INIReader::readArray<int>(const std::string &, const std::string &, const std::vector<int> &);
  // Explicit instantiation for std::string
  template std::vector<std::string> INIReader::readArray<std::string>(const std::string &, const std::string &, const std::vector<std::string> &);
}
