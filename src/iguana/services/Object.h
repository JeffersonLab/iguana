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
      Object(const std::string name = "");
      ~Object() {}

      /// Get the logger
      /// @return the logger used by this object
      std::unique_ptr<Logger>& Log();

      /// Change the name of this object
      /// @param name the new name
      void SetName(const std::string name);

      /// Get the name of this object
      std::string GetName() const;

      /// Set the log level to this level. Log messages with a lower level will not be printed.
      /// @see `Logger::Level` for available levels.
      /// @param lev the log level name
      void SetLogLevel(const std::string lev);

      /// Set the log level to this level. Log messages with a lower level will not be printed.
      /// @see `Logger::Level` for available levels.
      /// @param lev the log level
      void SetLogLevel(const Logger::Level lev);


    protected:

      /// The name of this object
      std::string m_name;

      /// `Logger` instance for this object
      std::unique_ptr<Logger> m_log;
  };
}
