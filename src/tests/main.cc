# include "iguana/Arbiter.h"
#include <iostream> // TODO: icwyu

int main(int argc, char **argv) {
  iguana::Arbiter arb;
  auto algo = arb.algo_map.at(iguana::Arbiter::clas12_FiducialCuts);
  algo->Start();
  std::cout << algo->Run(3,4) << std::endl;
  algo->Stop();
}
