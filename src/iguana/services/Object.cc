#include "Object.h"

namespace iguana {

  Object::Object(std::string_view name)
      : m_name(name)
  {}

  void Object::SetName(std::string_view name)
  {
    m_name = name;
  }

  std::string Object::GetName() const
  {
    return m_name;
  }

  void Object::SetLogLevel(std::string_view level)
  {
    m_log_settings.level = Logger::NameToLevel(level);
  }

  void Object::SetLogLevel(Logger::Level const level)
  {
    m_log_settings.level = level;
  }

  Logger::Level Object::GetLogLevel() const
  {
    return m_log_settings.level;
  }

  void Object::EnableLoggerStyle()
  {
    m_log_settings.styled = true;
  }

  void Object::DisableLoggerStyle()
  {
    m_log_settings.styled = false;
  }

}
