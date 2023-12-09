#include "iguana/AlgorithmSequence.h"
#include "algorithms/clas12/event_builder_filter/EventBuilderFilter.h"
#include "algorithms/clas12/lorentz_transformer/LorentzTransformer.h"
#include <hipo4/reader.h>

void printParticles(const std::string prefix, hipo::bank& b) {
  std::vector<int> pids;
  for(int row=0; row<b.getRows(); row++)
    pids.push_back(b.getInt("pid", row));
  fmt::print("{}: {}\n", prefix, fmt::join(pids, ", "));
}

int main(int argc, char **argv) {

  // parse arguments
  int argi = 1;
  const std::string inFileName = argc > argi ? std::string(argv[argi++]) : "data.hipo";
  const int         numEvents  = argc > argi ? std::stoi(argv[argi++])   : 1;

  // iguana algorithm sequence
  iguana::AlgorithmSequence seq;
  seq.Add<iguana::clas12::EventBuilderFilter>("pid_filter");
  seq.Add<iguana::clas12::LorentzTransformer>("mirror_frame");
  seq.PrintSequence();
  
  // set log levels
  seq.SetOption("pid_filter",   "log", "trace");
  seq.SetOption("mirror_frame", "log", "trace");

  // set algorithm options
  seq.SetOption("pid_filter",   "pids",      std::set<int>{11, 211, -211});
  seq.SetOption("pid_filter",   "testInt",   3);
  seq.SetOption("pid_filter",   "testFloat", 11.0);
  seq.SetOption("mirror_frame", "frame",     "mirror");

  // read input file
  hipo::reader reader(inFileName.c_str());

  // set banks
  hipo::banklist banks = reader.getBanks({
      "REC::Particle",
      "REC::Calorimeter"
      });
  enum banks_enum { // TODO: make this nicer
    b_particle,
    b_calo
  };

  seq.Start(banks);

  // event loop
  int iEvent = 0;
  while(reader.next(banks) && (iEvent++ < numEvents || numEvents == 0)) {
    printParticles("PIDS BEFORE algo->Run() ", banks.at(b_particle));
    seq.Run(banks);
    printParticles("PIDS AFTER algo->Run()  ", banks.at(b_particle));
  }

  // stop algorithms
  seq.Stop();
  return 0;
}
