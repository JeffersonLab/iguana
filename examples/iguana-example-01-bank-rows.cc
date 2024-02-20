#include <iguana/algorithms/clas12/EventBuilderFilter.h>
// #include <iguana/algorithms/clas12/MomentumCorrection.h> // FIXME
#include <hipo4/reader.h>

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

  // create the algorithms
  iguana::clas12::EventBuilderFilter algo_eventbuilder_filter;
  // iguana::clas12::MomentumCorrection algo_momentum_correction; // FIXME

  // set log levels
  algo_eventbuilder_filter.SetOption("log", "debug");
  // algo_momentum_correction.SetOption("log", "debug"); // FIXME

  // set algorithm options
  algo_eventbuilder_filter.SetOption<std::vector<int>>("pids", {11, 211, -211});

  // start the algorithms
  algo_eventbuilder_filter.Start();
  // algo_momentum_correction.Start(); // FIXME

  // run the algorithm sequence on each event
  int iEvent = 0;
  while(reader.next(banks) && (numEvents==0 || iEvent++ < numEvents)) {

    // show the particle bank
    auto& particleBank = banks.at(b_particle);
    particleBank.show();

    // loop over bank rows
    for(int row=0; row<particleBank.getRows(); row++) {

      // check the PID with EventBuilderFilter
      auto pid = particleBank.getInt("pid", row);
      if(algo_eventbuilder_filter.Filter(pid)) {

        // if accepted PID, correct its momentum
        /* FIXME: need momentum correction algo
        auto [px, py, pz, e] = algo_momentum_correction.Transform(
            particleBank.getFloat("px", row),
            particleBank.getFloat("py", row),
            particleBank.getFloat("pz", row),
            0.0 // (ignoring the energy)
            );

        // then print the result
        fmt::print("Accepted PID {}:\n", pid);
        auto printMomentum = [] (auto v1, auto v2) { fmt::print("  {:>20}  {:>20}\n", v1, v2); };
        printMomentum("p_old", "p_new");
        printMomentum("--------", "--------");
        printMomentum(particleBank.getFloat("px", row), px);
        printMomentum(particleBank.getFloat("py", row), py);
        printMomentum(particleBank.getFloat("pz", row), pz);
        */

      }
    }
  }

  // stop the algorithms
  algo_eventbuilder_filter.Stop();
  // algo_momentum_correction.Stop(); // FIXME
  return 0;
}
