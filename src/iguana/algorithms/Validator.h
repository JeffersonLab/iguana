#pragma once

#include <optional>
#include <mutex>

#include <hipo4/bank.h>

#include "iguana/algorithms/Algorithm.h"
#include "iguana/algorithms/AlgorithmSequence.h"

namespace iguana {

  /// @brief Base class for all algorithm validators to inherit from
  ///
  /// Similar to `iguana::Algorithm`, derived classes should override the methods
  /// `Validator::Start`, `Validator::Run` and `Validator::Stop`
  class Validator : public Algorithm
  {

    public:

      /// @param name the unique name for a derived class instance
      Validator(const std::string name="validator")
          : Algorithm(name)
          , m_output_dir("")
      {}
      virtual ~Validator() {}

      void Start(hipo::banklist& banks) override {};
      void Run(hipo::banklist& banks) const override {};
      void Stop() override {};

      /// Set this validator's output directory
      /// @param output_dir the output directory
      void SetOutputDirectory(std::string output_dir);

      /// Get this validator's output directory
      /// @returns an `optional`, which is set if the output directory is defined
      std::optional<std::string> GetOutputDirectory();

    protected:

      /// An `iguana::AlgorithmSequence` to be used for this validator
      std::unique_ptr<AlgorithmSequence> m_algo_seq;

      /// Mutex for locking procedures such as histogram filling in `Validator::Run`
      std::mutex m_mutex;

    private:

      /// output directory
      std::string m_output_dir;
  };
}
