#include "iguana/Arbiter.h"
#include <hipo4/reader.h>

int main(int argc, char **argv) {

  std::string inFile = "data.hipo";
  if(argc > 1) inFile = std::string(argv[1]);

  hipo::reader reader;
  reader.open(inFile.c_str());
  hipo::dictionary factory;
  reader.readDictionary(factory);
  factory.show();

  iguana::Arbiter arb;
  auto algo = arb.algo_map.at(iguana::Arbiter::clas12_FiducialCuts);
  algo->Start();
  fmt::print("test result: {}\n", algo->Run(3,4));
  algo->Stop();
}
