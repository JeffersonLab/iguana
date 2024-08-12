#pragma once

#include "Logger.h"

namespace iguana {

  /// @brief A named object with a `Logger` instance
  class Object
  {

    public:

      /// @param name the name of this object
      Object(std::string_view name = "");
      ~Object() {}

      /// Change the name of this object
      /// @param name the new name
      void SetName(std::string_view name);

      /// Get the name of this object
      std::string GetName() const;

      /// Set the log level to this level. Log messages with a lower level will not be printed.
      /// @see `Logger::Level` for available levels.
      /// @param level the log level name
      void SetLogLevel(std::string_view level);

      /// Set the log level to this level. Log messages with a lower level will not be printed.
      /// @see `Logger::Level` for available levels.
      /// @param level the log level
      void SetLogLevel(Logger::Level const level);

      /// @returns the current log level
      Logger::Level GetLogLevel() const;

      /// Enable styled log printouts, with color and emphasis
      void EnableLoggerStyle();

      /// Disable styled log printout color and emphasis
      void DisableLoggerStyle();

    protected:

      /// The name of this `Object`
      std::string m_name;

      /// `Logger` settings for this `Object` instance
      LoggerSettings m_log_settings;

  };
}
