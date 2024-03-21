// test configuration

#include <iguana/services/Logger.h>

inline int TestLogger()
{
  // this test just runs the `Logger` methods to catch any runtime errors
  std::vector<iguana::Logger> logs;
  logs.push_back({"styled_logger", iguana::Logger::Level::trace});
  logs.push_back({"unstyled_logger", iguana::Logger::Level::trace});

  // set styles
  logs.at(0).EnableStyle();
  logs.at(1).DisableStyle();

  // set non-existent level; should print errors
  auto non_existent_level = static_cast<iguana::Logger::Level>(1000);
  logs.at(0).SetLevel(non_existent_level);
  logs.at(0).SetLevel("non_existent_level");

  for(auto& log : logs) {
    // test all log levels
    log.Trace("trace is level {}", static_cast<int>(iguana::Logger::Level::trace));
    log.Debug("debug is level {}", static_cast<int>(iguana::Logger::Level::debug));
    log.Info("info is level {}", static_cast<int>(iguana::Logger::Level::info));
    log.Warn("warn is level {}", static_cast<int>(iguana::Logger::Level::warn));
    log.Error("error is level {}", static_cast<int>(iguana::Logger::Level::error));
    // test non-existent level
    log.Print(non_existent_level, "print to non-existent log level {}", static_cast<int>(non_existent_level));
    // test silence
    log.SetLevel("silent");
    log.Error("if this prints, 'silent' level failed");
    log.SetLevel("trace");
    // test run-time errors from `fmt`
    log.Info("too many arguments: {}", 1, 2); // noexcept
    try {
      log.Info("too few arguments: {} {}", 1);
    }
    catch(const std::exception& ex) {
      log.Info("too few arguments test threw expected exception");
    }
  }

  return 0;
}
