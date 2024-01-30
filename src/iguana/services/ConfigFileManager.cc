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
      for(const auto& dir : m_file_paths)
        m_log->Print(level, " - {}", dir);
      m_log->Print(level, "{:=^60}", "");
    }
  }

  std::string ConfigFileManager::FindFile(const std::string name) {
    m_log->Debug("Searching for file '{}' in...", name);
    for(const auto& dir : m_file_paths) {
      std::string filename = dir + "/" + name;
      auto found = std::filesystem::exists(filename);
      m_log->Debug("  ... {}{}", dir, found ? " - FOUND" : "");
      if(found)
        return filename;
    }
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
