/// @begin_doc_example
/// These are examples which demonstrate how to use Iguana in C++ analysis code. See each one for its usage guide.
/// @file iguana-example-00-basic.cc
/// @brief Example using full HIPO banks with Iguana This example requires the
/// user to have the C++ `hipo::bank` objects; see other examples if you do not have these banks in this format.
/// @par Usage
/// ```bash
/// iguana-example-00-basic [HIPO_FILE] [NUM_EVENTS]
///
///   HIPO_FILE   the HIPO file to analyze
///
///   NUM_EVENTS  the number of events to analyze;
///               set to zero to analyze all events
/// ```
/// @end_doc_example

#include <hipo4/reader.h>
#include <iguana/algorithms/AlgorithmSequence.h>

/// @brief show a bank along with a header
/// @param header the header to print above the bank
/// @param bank the bank to show
void prettyPrint(std::string header, hipo::bank& bank)
{
  fmt::print("{:=^70}\n", " " + header + " ");
  bank.show();
}

/// main function
int main(int argc, char** argv)
{

  // parse arguments
  int argi               = 1;
  char const* inFileName = argc > argi ? argv[argi++] : "data.hipo";
  int const numEvents    = argc > argi ? std::stoi(argv[argi++]) : 1;

  // read input file
  hipo::reader reader(inFileName);

  // set banks
  hipo::banklist banks = reader.getBanks({"RUN::config",
                                          "REC::Particle",
                                          "REC::Calorimeter",
                                          "REC::Track",
                                          "REC::Scintillator"});
  enum banks_enum { b_config,
                    b_particle }; // TODO: users shouldn't have to do this

  // iguana algorithm sequence
  iguana::AlgorithmSequence seq;
  seq.Add("clas12::EventBuilderFilter"); // filter by Event Builder PID
  seq.Add("clas12::SectorFinder"); // get the sector for each particle
  seq.Add("clas12::MomentumCorrection"); // momentum corrections

  // set log levels
  seq.SetOption("clas12::EventBuilderFilter", "log", "debug");
  seq.SetOption("clas12::MomentumCorrection", "log", "debug");

  // set algorithm options
  seq.SetOption<std::vector<int>>("clas12::EventBuilderFilter", "pids", {11, 211, -211});

  // start the algorithms
  seq.Start(banks);

  // run the algorithm sequence on each event
  int iEvent = 0;
  while(reader.next(banks) && (numEvents == 0 || iEvent++ < numEvents)) {
    prettyPrint("BEFORE", banks.at(b_particle));
    seq.Run(banks);
    prettyPrint("AFTER", banks.at(b_particle));
  }

  // stop algorithms
  seq.Stop();
  return 0;
}
