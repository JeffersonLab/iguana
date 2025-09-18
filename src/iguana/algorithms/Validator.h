#pragma once

#include <mutex>
#include <optional>

#include <hipo4/bank.h>

#include "iguana/algorithms/Algorithm.h"
#include "iguana/algorithms/AlgorithmSequence.h"

#ifdef IGUANA_ROOT_FOUND
#include <TStyle.h>
#endif

namespace iguana {

  /// @brief Base class for all algorithm validators to inherit from
  ///
  /// Similar to `iguana::Algorithm`, derived classes should override the methods
  /// `Validator::Start`, `Validator::Run` and `Validator::Stop`
  class Validator : public Algorithm
  {

    public:

      /// @param name the unique name for a derived class instance
      Validator(std::string_view name = "validator")
          : Algorithm(name)
          , m_output_dir("")
      {
#ifdef IGUANA_ROOT_FOUND
        // set styles for all validators' ROOT plots
        gStyle->SetOptStat(0);
        gStyle->SetPalette(55);
#endif
      }
      virtual ~Validator() {}

      void Start(hipo::banklist& banks) override {}
      bool Run(hipo::banklist& banks) const override { return true; }
      void Stop() override {}

      /// Set this validator's output directory
      /// @param output_dir the output directory
      void SetOutputDirectory(std::string_view output_dir);

      /// Get this validator's output directory
      /// @returns an `optional`, which is set if the output directory is defined
      std::optional<std::string> GetOutputDirectory();

    protected:

      /// An `iguana::AlgorithmSequence` to be used for this validator
      std::unique_ptr<AlgorithmSequence> m_algo_seq;

      /// Mutex for locking procedures such as histogram filling in `Validator::Run`
      mutable std::mutex m_mutex;

    private:

      /// output directory
      std::string m_output_dir;
  };
}
