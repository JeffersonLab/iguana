// ############################################################################
// # allow this header to be included only once
// ############################################################################
#pragma once

// ############################################################################
// # include `Algorithm.h`, which defines the base class `Algorithm`
// ############################################################################
#include "iguana/algorithms/Algorithm.h"

// ############################################################################
// # define the namespace
// # - all `iguana` code should be within the `iguana` namespace
// # - algorithms specific to an experiment or analysis may be put in a namespace within the `iguana` namespace
// #   - here we use the `iguana::example` namespace
// #   - for CLAS12-specific algorithms, use `iguana::clas12`
// ############################################################################
namespace iguana::example {

  // ############################################################################
  // # define the algorithm class
  // # - it must inherit from `Algorithm`, which includes common methods and objects used by each algorithm
  // # - it must also include a Doxygen docstring, typically defined by 3 forward slashes `///`
  // #   - algorithm classes have a standardized format for their docstring, please try to follow it and
  // #     keep it up-to-date with respect to the code
  // #   - see Doxygen documentation for more details, or see other algorithms
  // ############################################################################
  ///
  /// @brief_algo This is a template algorithm, used as an example showing how to write an algorithm.
  ///
  /// Provide a more detailed description of your algorithm here.
  ///
  /// @begin_doc_algo{Filter}
  /// @input_banks{REC::Particle}
  /// @output_banks{REC::Particle}
  /// @end_doc
  ///
  /// @begin_doc_config
  /// @config_param{exampleInt | int | an example `integer` configuration parameter}
  /// @config_param{exampleDouble | double | an example `double` configuration parameter}
  /// @end_doc
  class ExampleAlgorithm : public Algorithm
  {

      // ############################################################################
      // # this is a preprocessor macro call which generates boilerplate for the algorithm definition
      // # - the arguments are:
      // #   - the class name, `ExampleAlgorithm`
      // #   - a unique, "full" name of the algorithm, used by `AlgorithmFactory`; typically is the
      // #     namespace with the class name, excluding the `iguana::` part, but you are free to choose any name
      // # - NOTE: quotes are not used, and there is no need for a semicolon at the end of this call
      // # - see `../AlgorithmBoilerplate.h` for details
      // #   - the macros are relatively modular, so if you want to use your own constructor or destructor, you may
      // #     do so, and use other preprocessor macros called within `DEFINE_IGUANA_ALGORITHM` to complete
      // #     the boilerplate public and private functions and members
      // ############################################################################
      DEFINE_IGUANA_ALGORITHM(ExampleAlgorithm, example::ExampleAlgorithm)

    public:

      // ############################################################################
      // # define `Start`, `Run`, and `Stop` for this algorithm
      // # - each algorithm must have these methods (even if they do nothing)
      // ############################################################################
      void Start(hipo::banklist& banks) override;
      void Run(hipo::banklist& banks) const override;
      void Stop() override;

      // ############################################################################
      // # additional public functions go here
      // # - typically these are "action functions", which expose the primary operation of an algorithm
      // # - these functions are _unique_ to each algorithm, and therefore are not defined in the
      // #   `Algorithm` base class
      // # - their purpose is to allow the usage of the algorithm for users who _don't_ process full banks,
      // #   but rather process bank rows, or from some other data source
      // # - try to keep the parameters and return types _friendly_ to language bindings; for example,
      // #   avoid "complicated" types and lvalue references
      // # - don't forget the Doxygen docstrings
      // # - the action function here is trivial, just to show an example
      // # - you do not have to name it as `Filter`, but take a look at other algorithms and try to
      // #   keep some consistency, for example:
      // #   - `bool Filter` for a filtering type algorithm, such as fiducial cuts
      // #   - `Transform` for a transformation type algorithm, such as momentum corrections
      // #   - `Create` for a creation type algorithm, such as inclusive kinematic (x, Q2, etc.) reconstruction
      // ############################################################################
      /// **Action function**: checks if the PDG `pid` is positive;
      /// this is an example action function, please replace it with your own
      /// @param pid the particle PDG to check
      /// @returns `true` if `pid` is positive
      bool Filter(int const pid) const;

    private:

      // ############################################################################
      // # indices for the banks needed for this algorithm
      // # - see `Algorithm::GetBankIndex` for details
      // # - here, we just define one for the `REC::Particle` bank
      // # - convention: they should start with `b_`
      // ############################################################################
      /// `hipo::banklist` index for the particle bank (as an example)
      hipo::banklist::size_type b_particle;

      // ############################################################################
      // # configuration options
      // # - their type may be:
      // #   - one of the allowed types in `option_t`, which is a `std::variant`
      // #   - `std::set`, used by `Algorithm::GetOptionSet`, which converts
      // #     a user's `std::vector` option to a `std::set`
      // #   - your own type, but you will have to set it in the `Start()` method
      // # - here we show example `int` and `double` options
      // # - convention: they should start with `o_`
      // ############################################################################
      /// Example integer configuration option
      int o_exampleInt;
      /// Example double configuration option
      double o_exampleDouble;
  };

}
