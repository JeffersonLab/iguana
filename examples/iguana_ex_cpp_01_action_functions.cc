/// @begin_doc_example{cpp}
/// @file iguana_ex_cpp_01_action_functions.cc
/// @brief Example using Iguana action functions on data from bank rows. This is useful for
/// users who do not have the `hipo::bank` objects, instead only having the numerical data from them.
/// @par Usage
/// ```bash
/// iguana_ex_cpp_01_action_functions [HIPO_FILE] [NUM_EVENTS]
///
///   HIPO_FILE   the HIPO file to analyze
///
///   NUM_EVENTS  the number of events to analyze;
///               set to zero to analyze all events
/// ```
///
/// @note while this example _does_ use `hipo::bank` objects to read HIPO data, it demonstrates
/// using action functions called with the data _from_ these banks.
///
/// @end_doc_example
#include <hipo4/reader.h>
#include <iguana/algorithms/clas12/EventBuilderFilter/Algorithm.h>
#include <iguana/algorithms/clas12/MomentumCorrection/Algorithm.h>

/// main function
int main(int argc, char** argv)
{

  // parse arguments
  int argi               = 1;
  char const* inFileName = argc > argi ? argv[argi++] : "data.hipo";
  int const numEvents    = argc > argi ? std::stoi(argv[argi++]) : 1;

  // read input file
  hipo::reader reader(inFileName,{0});

  // set list of banks to be read
  hipo::banklist banks = reader.getBanks({"REC::Particle", "RUN::config"});

  // get bank index, for each bank we want to use after Iguana algorithms run
  auto b_particle = hipo::getBanklistIndex(banks, "REC::Particle");
  auto b_config   = hipo::getBanklistIndex(banks, "RUN::config");

  // set the concurrency model to single-threaded, since this example is single-threaded;
  // not doing this will use the thread-safe model, `"memoize"`
  iguana::GlobalConcurrencyModel = "single";

  // create the algorithms
  iguana::clas12::EventBuilderFilter algo_eventbuilder_filter;
  iguana::clas12::MomentumCorrection algo_momentum_correction;

  // set log levels
  algo_eventbuilder_filter.SetOption("log", "debug");
  algo_momentum_correction.SetOption("log", "debug");

  // set algorithm options
  algo_eventbuilder_filter.SetOption<std::vector<int>>("pids", {11, 211, -211});

  // start the algorithms
  algo_eventbuilder_filter.Start();
  algo_momentum_correction.Start();

  // run the algorithm sequence on each event
  int iEvent = 0;
  while(reader.next(banks) && (numEvents == 0 || iEvent++ < numEvents)) {

    // show the particle bank
    auto& particleBank = banks.at(b_particle);
    auto& configBank   = banks.at(b_config);
    particleBank.show();

    // loop over bank rows
    for(auto const& row : particleBank.getRowList()) {

      // check the PID with EventBuilderFilter
      auto pid = particleBank.getInt("pid", row);
      if(algo_eventbuilder_filter.Filter(pid)) {

        int sector = 1; // FIXME: get the sector number. The algorithm `clas12::SectorFinder` can do this, however
                        // it requires reading full `hipo::bank` objects, whereas this example is meant to demonstrate
                        // `iguana` usage operating _only_ on bank row elements

        // if accepted PID, correct its momentum
        auto [px, py, pz] = algo_momentum_correction.Transform(
            particleBank.getFloat("px", row),
            particleBank.getFloat("py", row),
            particleBank.getFloat("pz", row),
            sector,
            pid,
            configBank.getFloat("torus", 0));

        // then print the result
        fmt::print("Accepted PID {}:\n", pid);
        auto printMomentum = [](auto v1, auto v2)
        { fmt::print("  {:>20}  {:>20}\n", v1, v2); };
        printMomentum("p_old", "p_new");
        printMomentum("--------", "--------");
        printMomentum(particleBank.getFloat("px", row), px);
        printMomentum(particleBank.getFloat("py", row), py);
        printMomentum(particleBank.getFloat("pz", row), pz);
      }
    }
  }

  // stop the algorithms
  algo_eventbuilder_filter.Stop();
  algo_momentum_correction.Stop();
  return 0;
}
