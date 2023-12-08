#include "Object.h"

namespace iguana {

  Object::Object(const std::string name) :
    m_name(name),
    m_log(std::make_unique<Logger>(m_name))
  {}

  std::unique_ptr<Logger>& Object::Log() {
    return m_log;
  }

  void Object::SetName(const std::string name) {
    m_name = name;
    m_log->m_name = name;
  }

  std::string Object::GetName() const {
    return m_name;
  }

}
