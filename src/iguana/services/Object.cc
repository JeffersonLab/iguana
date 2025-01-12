#include "Object.h"

namespace iguana {

  Object::Object(std::string_view name, Logger::Level lev)
      : m_name(name)
      , m_log(std::make_unique<Logger>(m_name, lev))
      , m_spdlog(spdlog::default_logger()->clone(m_name))
  {
    m_spdlog->set_level(spdlog::level::trace);
  }

  std::unique_ptr<Logger>& Object::Log()
  {
    return m_log;
  }

  void Object::SetName(std::string_view name)
  {
    m_name        = name;
    m_log->m_name = name;
  }

  std::string Object::GetName() const
  {
    return m_name;
  }

  void Object::SetLogLevel(std::string_view lev)
  {
    m_log->SetLevel(lev);
  }

  void Object::SetLogLevel(Logger::Level const lev)
  {
    m_log->SetLevel(lev);
  }

  std::unique_ptr<Logger>& Object::GetLog()
  {
    return m_log;
  }
}
