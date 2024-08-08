#define WARN(fmt_str, ...) \
  if(m_level <= LogLevel::warn) \
    PrintLog( \
        stderr, \
        fmt::format("{}", fmt::styled(fmt::format("[warn] [{}]", m_name), fmt::emphasis::bold | fmt::fg(fmt::terminal_color::magenta))), \
        FMT_STRING(fmt_str), \
        __VA_ARGS__);

#define ERROR(fmt_str, ...) \
  if(m_level <= LogLevel::error) \
    PrintLog( \
        stderr, \
        fmt::format("{}", fmt::styled(fmt::format("[error] [{}]", m_name), fmt::emphasis::bold | fmt::fg(fmt::terminal_color::red))), \
        FMT_STRING(fmt_str), \
        __VA_ARGS__);
