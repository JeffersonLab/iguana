/// @begin_doc_example{cpp}
/// @file iguana_ex_cpp_00_run_functions_with_banks.cc
/// @brief Example using **full HIPO banks** with Iguana algorithms' `Run` functions, using `hipo::bank`
///
/// This example requires the user to have the C++ `hipo::bank` objects.
///
/// - see `iguana_ex_cpp_00_run_functions.cc` if you prefer `hipo::banklist`
/// - see other examples if you do not have `hipo::bank` objects
///
/// @par Usage
/// ```bash
/// iguana_ex_cpp_00_run_functions_with_banks [HIPO_FILE] [NUM_EVENTS]
///
///   HIPO_FILE   the HIPO file to analyze
///
///   NUM_EVENTS  the number of events to analyze;
///               set to zero to analyze all events
/// ```
/// @end_doc_example

#include <hipo4/reader.h>
#include <iguana/algorithms/clas12/EventBuilderFilter/Algorithm.h>
#include <iguana/algorithms/clas12/SectorFinder/Algorithm.h>
#include <iguana/algorithms/clas12/rga/MomentumCorrection/Algorithm.h>

/// main function
int main(int argc, char** argv)
{

  // parse arguments
  char const* inFileName = argc > 1 ? argv[1] : "data.hipo";
  int const numEvents    = argc > 2 ? std::stoi(argv[2]) : 3;

  // read input file
  hipo::reader reader(inFileName, {0});

  // set list of banks to be read
  hipo::dictionary dict;
  reader.readDictionary(dict);
  hipo::bank bank_config(dict.getSchema("RUN::config"));
  hipo::bank bank_particle(dict.getSchema("REC::Particle"));
  hipo::bank bank_calorimeter(dict.getSchema("REC::Calorimeter"));
  hipo::bank bank_track(dict.getSchema("REC::Track"));
  hipo::bank bank_scintillator(dict.getSchema("REC::Scintillator"));

  // iguana algorithm sequence
  // NOTE: unlike `iguana_ex_cpp_00_run_functions`, we do not use `AlgorithmSequence`, since
  // we'll be calling each algorithm's `Run(hipo::bank& bank1, ...)` functions, which are unique
  // for each algorithm (unlike `Run(hipo::banklist&)`
  iguana::clas12::EventBuilderFilter algo_eventbuilder_filter; // filter by Event Builder PID (a filter algorithm)
  iguana::clas12::SectorFinder algo_sector_finder; // get the sector for each particle (a creator algorithm)
  iguana::clas12::rga::MomentumCorrection algo_momentum_correction; // momentum corrections (a transformer algorithm)

  // set log levels
  // NOTE: this can also be done in a config file
  algo_eventbuilder_filter.SetLogLevel("info");
  algo_sector_finder.SetLogLevel("info");
  algo_momentum_correction.SetLogLevel("info");

  // set algorithm options
  // NOTE: this can also be done in a config file, but setting options here OVERRIDES config file settings
  // WARNING: in practice, verify the configuration parameter was set the way you want; configuration parameter values
  //          are printed out for algorithms at the "debug" log level
  algo_eventbuilder_filter.SetOption<std::vector<int>>("pids", {11, 211, -211});

  // start the algorithms
  algo_eventbuilder_filter.Start();
  algo_sector_finder.Start();
  algo_momentum_correction.Start();

  // define newly created bank object
  hipo::bank bank_sector = algo_sector_finder.GetCreatedBank();

  // run the algorithm sequence on each event
  int iEvent = 0;
  hipo::event event;
  while(reader.next() && (numEvents == 0 || iEvent++ < numEvents)) {

    // read the event's banks
    reader.read(event);
    event.getStructure(bank_config);
    event.getStructure(bank_particle);
    event.getStructure(bank_calorimeter);
    event.getStructure(bank_track);
    event.getStructure(bank_scintillator);

    // print the event number
    fmt::println("===== EVENT {} =====", bank_config.getInt("event", 0));

    // print the particle bank before Iguana algorithms
    fmt::println("----- BEFORE IGUANA -----");
    bank_particle.show(); // the original particle bank

    // run the sequence of Iguana algorithms, in your preferred order; continue
    // to the next event if any of the Run functions return `false`, which happens
    // if, for example, no particles pass a filter
    if(!algo_eventbuilder_filter.Run(bank_particle))
      continue;
    if(!algo_sector_finder.Run(bank_particle, bank_track, bank_calorimeter, bank_scintillator, bank_sector))
      continue;
    if(!algo_momentum_correction.Run(bank_particle, bank_sector, bank_config))
      continue;

    // print the banks after Iguana algorithms
    fmt::println("----- AFTER IGUANA -----");
    bank_particle.show(); // the filtered particle bank, with corrected momenta
    bank_sector.show(); // the new sector bank

    // print a table; first the header
    fmt::print("----- Analysis Particles -----\n");
    fmt::print("  {:<20} {:<20} {:<20} {:<20}\n", "row == pindex", "PDG", "|p|", "sector");
    // then print a row for each particle
    // - use the `hipo::bank::getRowList()` method to loop over the bank rows that PASS the filter
    // - if you'd rather loop over ALL bank rows, iterate from `i=0` up to `i < hipo::bank::getRows()` instead
    for(auto const& row : bank_particle.getRowList()) {
      auto p = std::hypot(
          bank_particle.getFloat("px", row),
          bank_particle.getFloat("py", row),
          bank_particle.getFloat("pz", row));
      auto pdg    = bank_particle.getInt("pid", row);
      auto sector = bank_sector.getInt("sector", row);
      fmt::print("  {:<20} {:<20} {:<20.3f} {:<20}\n", row, pdg, p, sector);
    }
    fmt::print("\n");
  }

  // stop algorithms
  algo_eventbuilder_filter.Stop();
  algo_sector_finder.Stop();
  algo_momentum_correction.Stop();
  return 0;
}
