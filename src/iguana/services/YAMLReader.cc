#include "YAMLReader.h"

namespace iguana {

  void YAMLReader::LoadFiles()
  {
    m_log->Debug("Called YAMLReader::LoadFiles()");
    for(const auto& file : m_files) {
      try {
        m_log->Debug(" - Loading YAML file: {}", file);
        m_configs.push_back({YAML::LoadFile(file), file}); // m_config must be the same ordering as m_files, so `push_back`
      }
      catch(const YAML::Exception& e) {
        m_log->Error(" - YAML Exception: {}", e.what());
      }
      catch(const std::exception& e) {
        m_log->Error(" - Exception: {}", e.what());
      }
    }
  }

}
