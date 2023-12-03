#include "iguana/Iguana.h"
#include <hipo4/reader.h>

void printParticles(std::string prefix, hipo::bank& b) {
  std::vector<int> pids;
  for(int row=0; row<b.getRows(); row++)
    pids.push_back(b.getInt("pid", row));
  fmt::print("{}: {}\n", prefix, fmt::join(pids, ", "));
}

int main(int argc, char **argv) {

  // parse arguments
  int argi = 1;
  std::string inFileName = argc > argi ? std::string(argv[argi++]) : "data.hipo";
  int         numEvents  = argc > argi ? std::stoi(argv[argi++])   : 3;

  // start iguana
  /* TODO: will be similified when we have more sugar in `iguana::Iguana`; until then we
   * use the test algorithm directly
   */
  iguana::Iguana I;
  auto& algo = I.algo_map.at(iguana::Iguana::clas12_EventBuilderFilter);
  algo->Log()->SetLevel("trace");
  // algo->Log()->DisableStyle();
  algo->SetOption("pids", std::set<int>{11, 211, -211});
  algo->SetOption("testInt", 3);
  algo->SetOption("testFloat", 11.0);
  algo->Start();

  /////////////////////////////////////////////////////

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

  // event loop
  int iEvent = 0;
  while(reader.next(banks) && (iEvent++ < numEvents || numEvents == 0)) {
    printParticles("PIDS BEFORE algo->Run() ", banks.at(b_particle));
    algo->Run(banks);
    printParticles("PIDS AFTER algo->Run()  ", banks.at(b_particle));
  }

  /////////////////////////////////////////////////////

  algo->Stop();
  return 0;
}
