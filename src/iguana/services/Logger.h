#include <fmt/format.h>
#include <fmt/color.h>
#include <fmt/ranges.h>

namespace iguana {

  class Logger {

    public:

      /// These are the available log levels, from lowest to highest:
      /// - `trace`: the most verbose level, used for fine-grained printouts for each event
      /// - `debug`: less verbose printout, expected to be less frequent than `trace`
      /// - `info`: the least verbose printout; this is the default level
      /// - `quiet`: use this level to only allow warnings and errors, silencing all other printouts
      /// - `warn`: an issue that may or may not be critical
      /// - `error`: an issue that is likely critical
      /// - `silent`: use this level to silence **all** printouts (use it at your own risk!)
      enum Level {
        trace,
        debug,
        info,
        quiet,
        warn,
        error,
        silent
      };

      /// @brief convert a `Level` name to an integer representation
      /// @param level the level name
      /// @returns the `Level` integer
      static Level NameToLevel(std::string_view level);

      /// @brief Print a log message
      /// @param out the output stream, _e.g._, `stdout` or `stderr`
      /// @param prefix a prefix in front of the log message, _e.g._, the log level or relevant file name
      /// @param fmt_str the `fmt` format string
      /// @param fmt_args the arguments for `fmt_str`
      template <typename... ARG_TYPES>
      static void PrintLog(FILE* out, std::string_view prefix, fmt::format_string<ARG_TYPES...> fmt_str, ARG_TYPES&&... fmt_args)
      {
        PrintLogV(out, prefix, fmt_str, fmt::make_format_args(fmt_args...));
      }

      /// Generate a header for a printout
      /// @param message the header message
      /// @param width the width of the header in number of characters
      /// @returns the header string
      static std::string Header(std::string_view message, int const width = 50);

    private:

      static void PrintLogV(FILE* out, std::string_view prefix, fmt::string_view fmt_str, fmt::format_args fmt_args);
  };

  /// `Logger` configuration settings
  struct LoggerSettings {
    /// The current `Logger` log level
    Logger::Level level{Logger::Level::info};
    /// Whether or not `Logger` printouts will be colored and formatted
    bool styled{true};
  };

}
