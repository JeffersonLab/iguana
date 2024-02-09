#include "ConfigFileManager.h"
#include <filesystem>

namespace iguana {

  ConfigFileManager::ConfigFileManager(const std::string name) : Object(name) {
    // add config files from installation prefix
    AddDirectory(GetConfigPrefix());
  }

  std::string ConfigFileManager::GetConfigPrefix() {
    return IGUANA_ETC;
  }

  void ConfigFileManager::AddDirectory(const std::string dir) {
    m_log->Debug("Add directory {}", dir);
    m_file_paths.push_front(dir);
  }

  void ConfigFileManager::PrintDirectories(const Logger::Level level) {
    if(m_log->GetLevel() <= level) {
      m_log->Print(level, "{:=^60}", " Configuration file search path order: ");
      m_log->Print(level, " - ./");
      for(const auto& dir : m_file_paths)
        m_log->Print(level, " - {}", dir);
      m_log->Print(level, "{:=^60}", "");
    }
  }

  std::string ConfigFileManager::FindFile(const std::string name) {
    m_log->Debug("Searching for file '{}' in:", name);
    // first try `./` or assume `name` is a relative or absolute path + filename
    auto found_local = std::filesystem::exists(name);
    m_log->Debug("  - ./{}", found_local ? " - FOUND" : "");
    if(found_local)
      return name;
    // then search each entry of `m_file_paths`
    for(const auto& dir : m_file_paths) {
      std::string filename = dir + "/" + name;
      auto found = std::filesystem::exists(filename);
      m_log->Debug("  - {}{}", dir, found ? " - FOUND" : "");
      if(found)
        return filename;
    }
    // throw exception if not found anywhere
    m_log->Error("Cannot find configuration file named '{}'", name);
    PrintDirectories(Logger::error);
    throw std::runtime_error("configuration file not found");
  }

  std::string ConfigFileManager::DirName(const std::string name) {
    auto result = std::filesystem::path{name}.parent_path().string();
    if(result=="")
      result = ".";
    return result;
  }

}
