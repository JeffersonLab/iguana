// ############################################################################
// # include the algorithm header
// ############################################################################
#include "ExampleAlgorithm.h"

// ############################################################################
// # namespace must match that in the header
// ############################################################################
namespace iguana::example {

  // ############################################################################
  // # boilerplate to register an algorithm, so `AlgorithmFactory` knows about it
  // # - this is a preprocessor macro call (see `../AlgorithmBoilerplate.h`)
  // # - this must be done here in the source file, not in the header
  // # - the argument is the class name; do not surround it with quotes
  // # - usage of a semicolon at the end is optional, but recommended
  // ############################################################################
  REGISTER_IGUANA_ALGORITHM(ExampleAlgorithm);

  // ############################################################################
  // # define `ExampleAlgorithm::Start()`
  // # - this overrides the virtual function `Algorithm::Start`
  // ############################################################################
  void ExampleAlgorithm::Start(hipo::banklist& banks)
  {
    // ############################################################################
    // # get configuration options
    // ############################################################################
    ParseYAMLConfig();
    o_exampleInt    = GetOptionScalar<int>("exampleInt");
    o_exampleDouble = GetOptionScalar<double>("exampleDouble");
    // ############################################################################
    // # get expected bank indices
    // # - here we make sure that parameter `banks` includes the banks that are
    // #   required to run this algorithm
    // # - we set the bank index values into the `b_*` members, to
    // #   avoid looking them up in the `Algorithm::Run` method
    // ############################################################################
    b_particle = GetBankIndex(banks, "REC::Particle");
  }


  // ############################################################################
  // # define `ExampleAlgorithm::Run()`
  // # - this overrides the virtual function `Algorithm::Run`
  // # - note that this method must be _thread safe_, for example, you cannot modify
  // #   class instance objects
  // # - try to avoid expensive operations here; instead, put them in the `Start` method
  // #   if it is reasonable to do so
  // ############################################################################
  void ExampleAlgorithm::Run(hipo::banklist& banks) const
  {
    // ############################################################################
    // # get the banks; here we just need `REC::Particle`
    // ############################################################################
    auto& particleBank = GetBank(banks, b_particle, "REC::Particle");
    // ############################################################################
    // # dump the bank
    // # - this will only happen if the log level for this algorithm is set low enough
    // # - this provides a look at the bank _before_ the algorithm runs
    // # - this is optional
    // ############################################################################
    ShowBank(particleBank, Logger::Header("INPUT PARTICLES"));

    // ############################################################################
    // # loop over the bank rows
    // ############################################################################
    for(int row = 0; row < particleBank.getRows(); row++) {
      // ############################################################################
      // # get the `pid` and feed it to the `Filter` action function; if the row
      // # is not acceptable, mask it out
      // ############################################################################
      auto pid    = particleBank.getInt("pid", row);
      auto accept = Filter(pid);
      if(!accept)
        MaskRow(particleBank, row);
      // ############################################################################
      // # print a useful debugging method (see `Logger.h` for details, or other
      // # algorithms for examples of how to use the logger)
      // ############################################################################
      m_log->Debug("input PID {} -- accept = {}", pid, accept);
    }

    // ############################################################################
    // # dump the modified bank (only if the log level is low enough); this is also optional
    // ############################################################################
    ShowBank(particleBank, Logger::Header("OUTPUT PARTICLES"));
  }


  // ############################################################################
  // # define the action function
  // ############################################################################
  bool ExampleAlgorithm::Filter(const int pid) const
  {
    return pid > 0;
  }


  // ############################################################################
  // # define `ExampleAlgorithm::Stop()`
  // # - this overrides the virtual function `Algorithm::Stop`
  // # - in this example, there is nothing to do
  // ############################################################################
  void ExampleAlgorithm::Stop()
  {
  }

}
