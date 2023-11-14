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

  hipo::bank particleBank(factory.getSchema("REC::Particle"));
  hipo::event event;

  iguana::Arbiter arb;
  auto algo = arb.algo_map.at(iguana::Arbiter::clas12_EventBuilderFilter);
  algo->Start();

  int count = 0;
  while(reader.next()) {
    if(count > 3) break;
    reader.read(event);
    event.getStructure(particleBank);

    auto resultBank = algo->Run({{"particles", particleBank}});

    fmt::print("BEFORE -> AFTER: {} -> {}\n", particleBank.getRows(), resultBank.at("particles").getRows());

    count++;
  }

  algo->Stop();
}
