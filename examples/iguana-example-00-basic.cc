#include <iguana/algorithms/AlgorithmSequence.h>
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
  seq.Add("clas12::EventBuilderFilter"); // filter by Event Builder PID
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
  while(reader.next(banks) && (numEvents==0 || iEvent++ < numEvents)) {
    prettyPrint("BEFORE", banks.at(b_particle));
    seq.Run(banks);
    prettyPrint("AFTER", banks.at(b_particle));
  }

  // stop algorithms
  seq.Stop();
  return 0;
}
