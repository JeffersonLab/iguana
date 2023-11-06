# include "iguana/Arbiter.h"

int main(int argc, char **argv) {
  iguana::Arbiter arb;
  auto algo = arb.algo_map.at(iguana::Arbiter::clas12_FiducialCuts);
  algo->Start();
  fmt::print("test result: {}\n", algo->Run(3,4));
  algo->Stop();
}
