/// @file
/// @brief `iguana::Logger` macros for printing log messages at different `iguana::Logger` levels.
///
/// @see `iguana::Logger::Level` for the list of log levels; each `Object` instance has its own log level,
/// which controls which log messages are printed
///
/// @warning These macros should:
/// - only be used in classes which inherit from `iguana::Object`
/// - only ever be included in `.cc` files and _NEVER_ in header `.h` files

// clang-format off

/// @brief print a detailed message at the most fine-grained log level; this is for things that are printed more frequently than by `DEBUG`,
/// for example, a printout of particle momenta for every particle
/// @param fmt_str the format string
/// @param ... the arguments for `fmt_str`
#define TRACE(fmt_str, ...) \
  { \
    if(m_log_settings.level <= Logger::Level::trace) \
      Logger::PrintLog( \
          stdout, \
          m_log_settings.styled ? fmt::format("{}", fmt::styled(fmt::format("[trace] [{}]", m_name), fmt::emphasis::bold)) : fmt::format("[trace] [{}]", m_name), \
          FMT_STRING(fmt_str), ##__VA_ARGS__); \
  }

/// @brief print a message useful for debugging; this is for things that are less-frequently printed than by `TRACE`,
/// for example, a printout indicating the beginning of a new event
/// @param fmt_str the format string
/// @param ... the arguments for `fmt_str`
#define DEBUG(fmt_str, ...) \
  { \
    if(m_log_settings.level <= Logger::Level::debug) \
      Logger::PrintLog( \
          stdout, \
          m_log_settings.styled ? fmt::format("{}", fmt::styled(fmt::format("[debug] [{}]", m_name), fmt::emphasis::bold)) : fmt::format("[debug] [{}]", m_name), \
          FMT_STRING(fmt_str), ##__VA_ARGS__); \
  }

/// @brief print an informational message; this is supposed to not be used for each event. For example, use this to indicate
/// an algorithm has been successfully configured
/// @param fmt_str the format string
/// @param ... the arguments for `fmt_str`
#define INFO(fmt_str, ...) \
  { \
    if(m_log_settings.level <= Logger::Level::info) \
      Logger::PrintLog( \
          stdout, \
          m_log_settings.styled ? fmt::format("{}", fmt::styled(fmt::format("[info] [{}]", m_name), fmt::emphasis::bold)) : fmt::format("[info] [{}]", m_name), \
          FMT_STRING(fmt_str), ##__VA_ARGS__); \
  }

/// @brief print warning message; these are errors that are not necessarily show-stoppers, but the user should still be informed of them
/// @param fmt_str the format string
/// @param ... the arguments for `fmt_str`
#define WARN(fmt_str, ...) \
  { \
    if(m_log_settings.level <= Logger::Level::warn) \
      Logger::PrintLog( \
          stderr, \
          m_log_settings.styled ? fmt::format("{}", fmt::styled(fmt::format("[warn] [{}]", m_name), fmt::emphasis::bold | fmt::fg(fmt::terminal_color::magenta))) : fmt::format("[warn] [{}]", m_name), \
          FMT_STRING(fmt_str), ##__VA_ARGS__); \
  }

/// @brief print an error message; this is for things that are truly issues and may cause failure of an algorithm
/// @param fmt_str the format string
/// @param ... the arguments for `fmt_str`
#define ERROR(fmt_str, ...) \
  { \
    if(m_log_settings.level <= Logger::Level::error) \
      Logger::PrintLog( \
          stderr, \
          m_log_settings.styled ? fmt::format("{}", fmt::styled(fmt::format("[error] [{}]", m_name), fmt::emphasis::bold | fmt::fg(fmt::terminal_color::red))) : fmt::format("[error] [{}]", m_name), \
          FMT_STRING(fmt_str), ##__VA_ARGS__); \
  }

/// @brief print a log message at a specific level
/// @param level the level to print at
/// @param fmt_str the format string
/// @param ... the arguments for `fmt_str`
#define PRINT_LOG(level, fmt_str, ...) \
  { \
    if(m_log_settings.level <= level) { \
      switch(level) { \
        case Logger::Level::trace: TRACE(fmt_str, ##__VA_ARGS__); break; \
        case Logger::Level::debug: DEBUG(fmt_str, ##__VA_ARGS__); break; \
        case Logger::Level::info:  INFO(fmt_str,  ##__VA_ARGS__); break; \
        case Logger::Level::warn:  WARN(fmt_str,  ##__VA_ARGS__); break; \
        case Logger::Level::error: ERROR(fmt_str, ##__VA_ARGS__); break; \
        default: throw std::runtime_error("called PRINT_LOG with bad log level"); \
      } \
    } \
  }

// clang-format on
