#include <hipo4/reader.h>
#include <iguana/algorithms/clas12/EventBuilderFilter.h>
#include <iguana/algorithms/clas12/MomentumCorrection.h>

int main(int argc, char** argv)
{

  // parse arguments
  int argi               = 1;
  const char* inFileName = argc > argi ? argv[argi++] : "data.hipo";
  const int numEvents    = argc > argi ? std::stoi(argv[argi++]) : 1;

  // read input file
  hipo::reader reader(inFileName);

  // set banks
  hipo::banklist banks = reader.getBanks({"REC::Particle", "RUN::config"});
  enum banks_enum { b_particle,
                    b_config }; // TODO: users shouldn't have to do this

  // create the algorithms
  iguana::clas12::EventBuilderFilter algo_eventbuilder_filter;
  iguana::clas12::MomentumCorrection algo_momentum_correction;

  // set log levels
  algo_eventbuilder_filter.SetOption("log", "debug");
  algo_momentum_correction.SetOption("log", "debug");

  // set algorithm options
  algo_eventbuilder_filter.SetOption<std::vector<int>>("pids", {11, 211, -211});

  // start the algorithms
  algo_eventbuilder_filter.Start();
  algo_momentum_correction.Start();

  // run the algorithm sequence on each event
  int iEvent = 0;
  while(reader.next(banks) && (numEvents == 0 || iEvent++ < numEvents)) {

    // show the particle bank
    auto& particleBank = banks.at(b_particle);
    auto& configBank   = banks.at(b_config);
    particleBank.show();

    // loop over bank rows
    for(int row = 0; row < particleBank.getRows(); row++) {

      // check the PID with EventBuilderFilter
      auto pid = particleBank.getInt("pid", row);
      if(algo_eventbuilder_filter.Filter(pid)) {

        int sector = 1; // FIXME: get the sector number

        // if accepted PID, correct its momentum
        auto [px, py, pz] = algo_momentum_correction.Transform(
            particleBank.getFloat("px", row),
            particleBank.getFloat("py", row),
            particleBank.getFloat("pz", row),
            sector,
            pid,
            configBank.getFloat("torus", 0));

        // then print the result
        fmt::print("Accepted PID {}:\n", pid);
        auto printMomentum = [](auto v1, auto v2)
        { fmt::print("  {:>20}  {:>20}\n", v1, v2); };
        printMomentum("p_old", "p_new");
        printMomentum("--------", "--------");
        printMomentum(particleBank.getFloat("px", row), px);
        printMomentum(particleBank.getFloat("py", row), py);
        printMomentum(particleBank.getFloat("pz", row), pz);
      }
    }
  }

  // stop the algorithms
  algo_eventbuilder_filter.Stop();
  algo_momentum_correction.Stop();
  return 0;
}
