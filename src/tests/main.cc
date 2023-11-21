#include "iguana/Iguana.h"
#include <hipo4/reader.h>

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
  auto algo = I.algo_map.at(iguana::Iguana::clas12_EventBuilderFilter);
  algo->Start();

  /////////////////////////////////////////////////////

  // read input file
  hipo::reader reader;
  reader.open(inFileName.c_str());

  // get bank schema
  /* TODO: users should not have to do this; this is a workaround until
   * the pattern `hipo::event::getBank("REC::Particle")` is possible
   */
  hipo::dictionary factory;
  reader.readDictionary(factory);
  auto particleBank = std::make_shared<hipo::bank>(factory.getSchema("REC::Particle"));

  // event loop
  hipo::event event;
  int iEvent = 0;
  while(reader.next(event) && (iEvent++ < numEvents || numEvents == 0)) {
    event.getStructure(*particleBank);
    auto resultBank = algo->Run({{"particles", particleBank}});
  }

  /////////////////////////////////////////////////////

  algo->Stop();
  return 0;
}
