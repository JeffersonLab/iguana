#include <TSystem.h>
#include <iguana/algorithms/physics/InclusiveKinematics/Algorithm.h>

void iguana_example_ROOT_macro() {

  // load the iguana algorithms library
  gSystem->Load("libIguanaAlgorithms");

  // run the inclusive kinematics action function
  iguana::physics::InclusiveKinematics algo;
  algo.Start();
  auto result = algo.ComputeFromLepton(0.3, 0.3, 5.0);
  std::cout << "kinematics:"
    << "\n Q2 = " << result.Q2
    << "\n  x = " << result.x
    << "\n  W = " << result.W
    << std::endl;
}
