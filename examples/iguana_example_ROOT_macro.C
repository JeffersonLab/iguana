/// @begin_doc_example{cpp}
/// @file iguana_example_ROOT_macro.C
/// @brief Very simple example showing how to load and use an Iguana algorithm in a ROOT macro; this example
/// only runs the algorithm and does not require any input data.
/// @par Usage
/// ```bash
/// root -b -q iguana_example_ROOT_macro.C
/// ```
/// @end_doc_example

#include <TSystem.h>
#include <iguana/algorithms/physics/InclusiveKinematics/Algorithm.h>

/// example ROOT macro function
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
