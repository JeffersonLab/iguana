#pragma once

#include <memory>
#include <string>

#include "Logger.h"

namespace iguana {

  /// @brief A named object with a `Logger` instance
  class Object
  {

    public:

      /// @param name the name of this object
      /// @param lev the log level
      Object(std::string_view name = "", Logger::Level lev = Logger::DEFAULT_LEVEL);
      ~Object() {}

      /// Get the logger
      /// @return the logger used by this object
      std::unique_ptr<Logger>& Log();

      /// Change the name of this object
      /// @param name the new name
      void SetName(std::string_view name);

      /// @returns the name of this object
      std::string GetName() const;

      /// Set the log level to this level. Log messages with a lower level will not be printed.
      /// @see `Logger::Level` for available levels.
      /// @param lev the log level name
      void SetLogLevel(std::string_view lev);

      /// Set the log level to this level. Log messages with a lower level will not be printed.
      /// @see `Logger::Level` for available levels.
      /// @param lev the log level
      void SetLogLevel(Logger::Level const lev);

      /// @returns reference to this object's logger
      std::unique_ptr<Logger>& GetLog();


    protected:

      /// The name of this object
      std::string m_name;

      /// `Logger` instance for this object
      std::unique_ptr<Logger> m_log;
  };
}
