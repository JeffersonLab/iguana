/// @begin_doc_example{cpp}
/// @file iguana_ex_cpp_00_run_functions.cc
/// @brief Example using **full HIPO banks** with Iguana algorithms' `Run` functions. This example requires the
/// user to have the C++ `hipo::bank` objects; see other examples if you do not have banks in this format.
/// @par Usage
/// ```bash
/// iguana_ex_cpp_00_run_functions [HIPO_FILE] [NUM_EVENTS]
///
///   HIPO_FILE   the HIPO file to analyze
///
///   NUM_EVENTS  the number of events to analyze;
///               set to zero to analyze all events
/// ```
/// @end_doc_example

#include <hipo4/reader.h>
#include <iguana/algorithms/AlgorithmSequence.h>

/// main function
int main(int argc, char** argv)
{

  // parse arguments
  char const* inFileName = argc > 1 ? argv[1] : "data.hipo";
  int const numEvents    = argc > 2 ? std::stoi(argv[2]) : 3;

  // read input file
  hipo::reader reader(inFileName,{0});

  // set list of banks to be read
  hipo::banklist banks = reader.getBanks({"RUN::config",
                                          "REC::Particle",
                                          "REC::Calorimeter",
                                          "REC::Track",
                                          "REC::Scintillator"});

  // iguana algorithm sequence
  // NOTE: the order that they are added to the sequence here will be the same order in which they will be run
  iguana::AlgorithmSequence seq;
  seq.Add("clas12::EventBuilderFilter"); // filter by Event Builder PID (a filter algorithm)
  seq.Add("clas12::SectorFinder"); // get the sector for each particle (a creator algorithm)
  seq.Add("clas12::MomentumCorrection"); // momentum corrections (a transformer algorithm)
  // seq.PrintSequence();

  // set log levels
  // NOTE: this can also be done in a config file
  seq.SetOption("clas12::EventBuilderFilter", "log", "info");
  seq.SetOption("clas12::SectorFinder", "log", "info");
  seq.SetOption("clas12::MomentumCorrection", "log", "info");

  // set algorithm options
  // NOTE: this can also be done in a config file, but setting options here OVERRIDES config file settings
  seq.SetOption<std::vector<int>>("clas12::EventBuilderFilter", "pids", {11, 211, -211});

  // start the algorithms
  seq.Start(banks);

  // get bank index, for each bank we want to use after Iguana algorithms run
  // NOTE: new banks from creator algorithms are initialized by `Start`
  auto b_config   = hipo::getBanklistIndex(banks, "RUN::config");
  auto b_particle = hipo::getBanklistIndex(banks, "REC::Particle");
  auto b_sector   = hipo::getBanklistIndex(banks, "REC::Particle::Sector"); // new created bank

  // run the algorithm sequence on each event
  int iEvent = 0;
  while(reader.next(banks) && (numEvents == 0 || iEvent++ < numEvents)) {

    // references to this event's banks
    auto& bank_config   = banks.at(b_config);
    auto& bank_particle = banks.at(b_particle);
    auto& bank_sector   = banks.at(b_sector);

    // print the event number
    fmt::println("===== EVENT {} =====", bank_config.getInt("event", 0));

    // print the particle bank before Iguana algorithms
    fmt::println("----- BEFORE IGUANA -----");
    bank_particle.show(); // the original particle bank

    // run the sequence of Iguana algorithms
    seq.Run(banks);

    // print the banks after Iguana algorithms
    fmt::println("----- AFTER IGUANA -----");
    bank_particle.show(); // the filtered particle bank, with corrected momentum
    bank_sector.show();   // the new sector bank

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
      auto pdg = bank_particle.getInt("pid", row);
      auto sector = bank_sector.getInt("sector", row);
      fmt::print("  {:<20} {:<20} {:<20.3f} {:<20}\n", row, pdg, p, sector);
    }
    fmt::print("\n");

  }

  // stop algorithms
  seq.Stop();
  return 0;
}
