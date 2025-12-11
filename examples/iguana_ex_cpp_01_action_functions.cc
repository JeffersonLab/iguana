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
/// using action functions called with the data _from_ these banks. We only use `hipo::bank` to
/// _obtain_ these data, since it is convenient that we don't have to use another HIPO reader,
/// which would introduce another build dependency to this program.
///
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
  hipo::banklist banks = reader.getBanks({"REC::Particle",
                                          "RUN::config",
                                          "REC::Track",
                                          "REC::Calorimeter",
                                          "REC::Scintillator"});

  // get bank index, for each bank we want to use after Iguana algorithms run
  auto b_particle     = iguana::tools::GetBankIndex(banks, "REC::Particle");
  auto b_config       = iguana::tools::GetBankIndex(banks, "RUN::config");
  auto b_track        = iguana::tools::GetBankIndex(banks, "REC::Track");
  auto b_calorimeter  = iguana::tools::GetBankIndex(banks, "REC::Calorimeter");
  auto b_scintillator = iguana::tools::GetBankIndex(banks, "REC::Scintillator");

  // set the concurrency model to single-threaded, since this example is single-threaded;
  // not doing this will use the thread-safe model, `"memoize"`
  iguana::GlobalConcurrencyModel = "single";

  // create the algorithms
  iguana::clas12::EventBuilderFilter algo_eventbuilder_filter; // filter by Event Builder PID (a filter algorithm)
  iguana::clas12::SectorFinder algo_sector_finder; // get the sector for each particle (a creator algorithm)
  iguana::clas12::rga::MomentumCorrection algo_momentum_correction; // momentum corrections (a transformer algorithm)

  // set log levels
  algo_eventbuilder_filter.SetOption("log", "info");
  algo_sector_finder.SetOption("log", "info");
  algo_momentum_correction.SetOption("log", "info");

  // set algorithm options
  // NOTE: this can also be done in a config file
  algo_eventbuilder_filter.SetOption<std::vector<int>>("pids", {11, 211, -211});

  // start the algorithms
  algo_eventbuilder_filter.Start();
  algo_sector_finder.Start();
  algo_momentum_correction.Start();

  // run the algorithms on each event
  int iEvent = 0;
  while(reader.next(banks) && (numEvents == 0 || iEvent++ < numEvents)) {

    // get the banks for this event
    auto& particleBank     = banks.at(b_particle);
    auto& configBank       = banks.at(b_config);
    auto& trackBank        = banks.at(b_track);
    auto& calorimeterBank  = banks.at(b_calorimeter);
    auto& scintillatorBank = banks.at(b_scintillator);

    // show the particle bank
    // particleBank.show();

    // print the event number
    fmt::print("evnum = {}\n", configBank.getInt("event", 0));

    // we'll need information from all the rows of REC::Track,Calorimeter,Scintilator,
    // in order to get the sector information for each particle
    // FIXME: there are vectorized accessors, but we cannot use them yet; see https://github.com/gavalian/hipo/issues/72
    //        until then, we fill `std::vector`s manually
    std::vector<int> trackBank_sectors;
    std::vector<int> trackBank_pindices;
    std::vector<int> calorimeterBank_sectors;
    std::vector<int> calorimeterBank_pindices;
    std::vector<int> scintillatorBank_sectors;
    std::vector<int> scintillatorBank_pindices;
    for(auto const& r : trackBank.getRowList()) {
      trackBank_sectors.push_back(trackBank.getByte("sector", r));
      trackBank_pindices.push_back(trackBank.getShort("pindex", r));
    }
    for(auto const& r : calorimeterBank.getRowList()) {
      calorimeterBank_sectors.push_back(calorimeterBank.getByte("sector", r));
      calorimeterBank_pindices.push_back(calorimeterBank.getShort("pindex", r));
    }
    for(auto const& r : scintillatorBank.getRowList()) {
      scintillatorBank_sectors.push_back(scintillatorBank.getByte("sector", r));
      scintillatorBank_pindices.push_back(scintillatorBank.getShort("pindex", r));
    }

    // loop over bank rows
    for(auto const& row : particleBank.getRowList()) {

      // check the PID with EventBuilderFilter
      auto pid = particleBank.getInt("pid", row);
      if(algo_eventbuilder_filter.Filter(pid)) {

        // get the sector for this particle; this is using a vector action function, so
        // many of its arguments are of type `std::vector`
        auto sector = algo_sector_finder.GetStandardSector(
            trackBank_sectors,
            trackBank_pindices,
            calorimeterBank_sectors,
            calorimeterBank_pindices,
            scintillatorBank_sectors,
            scintillatorBank_pindices,
            row);

        // correct the particle momentum
        auto [px, py, pz] = algo_momentum_correction.Transform(
            particleBank.getFloat("px", row),
            particleBank.getFloat("py", row),
            particleBank.getFloat("pz", row),
            sector,
            pid,
            configBank.getFloat("torus", 0));

        // then print the result
        fmt::print("Analysis Particle PDG = {}\n", pid);
        fmt::print("  sector = {}\n", sector);
        fmt::print("  p_old = ({:11.5f}, {:11.5f}, {:11.5f})\n", particleBank.getFloat("px", row), particleBank.getFloat("py", row), particleBank.getFloat("pz", row));
        fmt::print("  p_new = ({:11.5f}, {:11.5f}, {:11.5f})\n", px, py, pz);
      }
    }
  }

  // stop the algorithms
  algo_eventbuilder_filter.Stop();
  algo_sector_finder.Stop();
  algo_momentum_correction.Stop();
  return 0;
}
