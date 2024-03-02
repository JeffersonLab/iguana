#pragma once

#include <optional>

#include <hipo4/bank.h>

#include "iguana/algorithms/ValidatorBoilerplate.h"
#include "iguana/services/Object.h"

namespace iguana {

  /// @brief Base class for all algorithm validators to inherit from
  ///
  /// Similar to `iguana::Algorithm`, derived classes should override the methods
  /// `Validator::Start`, `Validator::Run` and `Validator::Stop`
  class Validator : public Object
  {

    public:

      /// @param name the unique name for a derived class instance
      Validator(const std::string name)
          : Object(name)
      {}
      virtual ~Validator() {}

      /// Initialize a validator before any events are processed
      /// @param banks the list of banks this validator will use, so that `Validator::Run` can cache the indices
      ///        of the banks that it needs
      /// @param output_dir if non-empty, validator output files will be written to this directory
      virtual void Start(hipo::banklist& banks) = 0;

      /// Run a validator for an event. Note that unlike `iguana::Algorithm::Run`, this is *NOT* `const` qualified,
      /// so that this method may be used to fill histograms, for example.
      /// @param banks the list of banks to process
      virtual void Run(hipo::banklist& banks) = 0;

      /// Finalize a validator after all events are processed, _e.g._, write output files
      virtual void Stop() = 0;

      /// Set this validator's output directory
      /// @param output_dir the output directory
      void SetOutputDirectory(std::string output_dir);

      /// Get this validator's output directory
      /// @returns an `optional`, which is set if the output directory is defined
      std::optional<std::string> GetOutputDirectory();

    private:

      /// output directory
      std::string m_output_dir;
  };

  //////////////////////////////////////////////////////////////////////////////

  /// Validator pointer type
  using validator_t = std::unique_ptr<Validator>;

  /// @brief Factory to create a validator.
  class ValidatorFactory
  {

    public:

      /// Validator creator function type
      using validator_creator_t = std::function<validator_t()>;

      ValidatorFactory() = delete;

      /// Register a validator with a unique name. Validators register themselves by calling this function.
      /// @param name the name of the validator (not equivalent to `Object::m_name`)
      /// @param creator the creator function
      static bool Register(const std::string& name, validator_creator_t creator) noexcept;

      /// Create a validator.
      /// @param name the name of the validator, which was used as an argument in the `ValidatorFactory::Register` call
      static validator_t Create(const std::string& name) noexcept;

    private:

      /// Association between the validator names and their creators
      static std::unordered_map<std::string, validator_creator_t> s_creators;
  };
}
