#include "Object.h"

namespace iguana {

  Object::Object(std::string_view name)
      : m_name(name)
      , m_log(std::make_unique<Logger>(m_name))
  {}

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

  void Object::SetLogLevel(const Logger::Level lev)
  {
    m_log->SetLevel(lev);
  }

}
