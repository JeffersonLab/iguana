#include <iostream>
#include <iguana/algorithms/physics/InclusiveKinematics.h>

void iguana_example_ROOT_macro() {
  iguana::physics::InclusiveKinematics algo;
  algo.Start();
  auto result = algo.ComputeFromLepton(0.3, 0.3, 5.0);
  std::cout << result.Q2 << std::endl;
}

