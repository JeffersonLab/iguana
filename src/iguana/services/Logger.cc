#include "Logger.h"

namespace iguana {

  Logger::Logger(const std::string name, const Level lev, bool const enable_style)
      : m_name(name)
      , m_enable_style(enable_style)
  {
    m_level_names = {
        {trace, "trace"},
        {debug, "debug"},
        {info, "info"},
        {quiet, "quiet"},
        {warn, "warn"},
        {error, "error"},
        {silent, "silent"}};
    SetLevel(lev);
  }

  void Logger::SetLevel(const std::string lev)
  {
    for(auto& [lev_i, lev_n] : m_level_names) {
      if(lev == lev_n) {
        m_level = lev_i;
        Debug("log level set to '{}'", lev);
        return;
      }
    }
    Error("Log level '{}' is not a known log level; the log level will remain at '{}'", lev, m_level_names.at(m_level));
  }

  void Logger::SetLevel(const Level lev)
  {
    try {
      auto level_name = m_level_names.at(lev);
      m_level         = lev;
      Debug("log level set to '{}'", level_name);
    }
    catch(std::out_of_range const& ex) {
      Error("Log level '{}' is not a known log level; the log level will remain at '{}'", static_cast<int>(lev), m_level_names.at(m_level));
    }
  }

  Logger::Level Logger::GetLevel()
  {
    return m_level;
  }

  void Logger::EnableStyle()
  {
    m_enable_style = true;
  }

  void Logger::DisableStyle()
  {
    m_enable_style = false;
  }

  std::string Logger::Header(const std::string message, int const width)
  {
    return fmt::format("{:=^{}}", " " + message + " ", width);
  }

}
