#include <iguana/algorithms/clas12/EventBuilderFilter.h>
#include <hipo4/reader.h>

int main(int argc, char **argv) {

  /* DISABLED until `run_banks` is more stable

  // parse arguments
  int argi = 1;
  std::string inFileName = argc > argi ? std::string(argv[argi++]) : "data.hipo";
  int         numEvents  = argc > argi ? std::stoi(argv[argi++])   : 3;


  // start the algorithm
  auto algo = std::make_shared<iguana::clas12::EventBuilderFilter>();
  algo->SetOption("pids", std::set<int>{11, 211, -211});
  algo->Start();

  /////////////////////////////////////////////////////

  // read input file
  hipo::reader reader;
  reader.open(inFileName.c_str());

  // get bank schema
  hipo::dictionary factory;
  reader.readDictionary(factory);
  auto particleBank = std::make_shared<hipo::bank>(factory.getSchema("REC::Particle"));

  // event loop
  hipo::event event;
  int iEvent = 0;
  while(reader.next(event) && (iEvent++ < numEvents || numEvents == 0)) {
    event.getStructure(*particleBank);
    fmt::print("PIDS FILTERED BY algo->Filter():\n");
    for(int row=0; row<particleBank->getRows(); row++) {
      auto pid = particleBank->getInt("pid", row);
      fmt::print("{:>10}:{}\n", pid, algo->Filter(pid) ? " -- ACCEPT" : "");
    }

  }

  /////////////////////////////////////////////////////

  algo->Stop();
  */
  return 0;
}
