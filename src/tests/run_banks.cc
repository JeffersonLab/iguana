#include "algorithms/AlgorithmSequence.h"
#include "algorithms/clas12/event_builder_filter/EventBuilderFilter.h"
#include "algorithms/clas12/lorentz_transformer/LorentzTransformer.h"
#include <hipo4/reader.h>

// show a bank along with a header
void prettyPrint(std::string header, hipo::bank& bank) {
  fmt::print("{:=^70}\n", " "+header+" ");
  bank.show();
}

int main(int argc, char **argv) {

  // parse arguments
  int argi = 1;
  const char* inFileName = argc > argi ? argv[argi++]            : "data.hipo";
  const int   numEvents  = argc > argi ? std::stoi(argv[argi++]) : 1;

  // read input file
  hipo::reader reader(inFileName);

  // set banks
  hipo::banklist banks = reader.getBanks({ "REC::Particle", "REC::Calorimeter" });
  enum banks_enum { b_particle, b_calo }; // TODO: users shouldn't have to do this

  // iguana algorithm sequence
  iguana::AlgorithmSequence seq;
  seq.Add<iguana::clas12::EventBuilderFilter>("pid_filter"); // filter by Event Builder PID
  seq.Add<iguana::clas12::LorentzTransformer>("new_frame");  // Lorentz transform the momenta
  
  // set log levels
  seq.SetOption("pid_filter", "log", "debug");
  seq.SetOption("new_frame",  "log", "debug");

  // set algorithm options
  seq.SetOption("pid_filter", "pids",  std::set<int>{11, 211, -211});
  seq.SetOption("new_frame",  "frame", "mirror");

  // start the algorithms
  seq.Start(banks);

  // run the algorithm sequence on each event
  int iEvent = 0;
  while(reader.next(banks) && (numEvents==0 || iEvent++ < numEvents)) {
    prettyPrint("BEFORE", banks.at(b_particle));
    seq.Run(banks);
    prettyPrint("AFTER", banks.at(b_particle));
  }

  // stop algorithms
  seq.Stop();
  return 0;
}
