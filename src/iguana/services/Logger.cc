#include "Logger.h"

namespace iguana {

  Logger::Level Logger::NameToLevel(std::string_view level)
  {
    if(level == "trace") return trace;
    else if(level == "debug") return debug;
    else if(level == "info") return info;
    else if(level == "quiet") return quiet;
    else if(level == "warn") return warn;
    else if(level == "error") return error;
    else if(level == "silent") return silent;
    throw std::runtime_error(fmt::format("unkown log level {:?}", level));
  }

  std::string Logger::Header(std::string_view message, int const width)
  {
    return fmt::format("{:=^{}}", fmt::format(" {} ", message), width);
  }

  void Logger::PrintLogV(FILE* out, std::string_view prefix, fmt::string_view fmt_str, fmt::format_args fmt_args)
  {
    fmt::print(out, "{} {}\n", prefix, fmt::vformat(fmt_str, fmt_args));
  }

}
