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
  int argi               = 1;
  char const* inFileName = argc > argi ? argv[argi++] : "data.hipo";
  int const numEvents    = argc > argi ? std::stoi(argv[argi++]) : 1;

  // read input file
  hipo::reader reader(inFileName,{0});

  // set list of banks to be read
  hipo::banklist banks = reader.getBanks({"RUN::config",
                                          "REC::Particle",
                                          "REC::Calorimeter",
                                          "REC::Track",
                                          "REC::Scintillator"});

  // set the concurrency model to single-threaded, since this example is single-threaded;
  // not doing this will use the thread-safe model, `"memoize"`
  iguana::GlobalConcurrencyModel = "single";

  // iguana algorithm sequence
  iguana::AlgorithmSequence seq;
  seq.Add("clas12::EventBuilderFilter"); // filter by Event Builder PID (a filter algorithm)
  seq.Add("clas12::SectorFinder"); // get the sector for each particle (a creator algorithm)
  seq.Add("clas12::MomentumCorrection"); // momentum corrections (a transformer algorithm)

  // set log levels
  // NOTE: this can also be done in a config file
  seq.SetOption("clas12::EventBuilderFilter", "log", "info");
  seq.SetOption("clas12::SectorFinder", "log", "info");
  seq.SetOption("clas12::MomentumCorrection", "log", "info");

  // set algorithm options
  // NOTE: this can also be done in a config file
  seq.SetOption<std::vector<int>>("clas12::EventBuilderFilter", "pids", {11, 211, -211});

  // start the algorithms
  seq.Start(banks);

  // get bank index, for each bank we want to use after Iguana algorithms run
  // NOTE: new banks from creator algorithms are initialized by `Start`
  auto b_particle = hipo::getBanklistIndex(banks, "REC::Particle");
  auto b_sector   = hipo::getBanklistIndex(banks, "REC::Particle::Sector"); // new created bank

  // run the algorithm sequence on each event
  int iEvent = 0;
  while(reader.next(banks) && (numEvents == 0 || iEvent++ < numEvents)) {

    // print the particle bank before Iguana algorithms
    fmt::print("{:=^70}\n", " BEFORE IGUANA ");
    banks.at(b_particle).show();

    // run the sequence of Iguana algorithms
    seq.Run(banks);

    // print the banks after Iguana algorithms
    fmt::print("{:=^70}\n", " AFTER IGUANA ");
    banks.at(b_particle).show();
    banks.at(b_sector).show();

    // print
    fmt::print("Analysis Particles:\n");
    fmt::print("  {:<20} {:<20} {:<20} {:<20}\n", "row == pindex", "PDG", "|p|", "sector");
    for(auto const& row : banks.at(b_particle).getRowList()) {
      auto p = std::hypot(
          banks.at(b_particle).getFloat("px", row),
          banks.at(b_particle).getFloat("py", row),
          banks.at(b_particle).getFloat("pz", row));
      auto pdg = banks.at(b_particle).getInt("pid", row);
      auto sector = banks.at(b_sector).getInt("sector", row);
      fmt::print("  {:<20} {:<20} {:<20.3} {:<20}\n", row, pdg, p, sector);
    }
    fmt::print("\n");

  }

  // stop algorithms
  seq.Stop();
  return 0;
}
