/// @begin_doc_example{cpp}
/// @file iguana_ex_cpp_ROOT_macro.C
/// @brief Very simple example showing how to load and use an Iguana algorithm in a ROOT macro; this example
/// only runs the algorithm and does not require any input data.
/// @par Usage
/// ```bash
/// root -b -q iguana_ex_cpp_ROOT_macro.C
/// ```
/// @end_doc_example

#include <TSystem.h>
#include <iguana/algorithms/physics/InclusiveKinematics/Algorithm.h>

/// example ROOT macro function
void iguana_ex_cpp_ROOT_macro() {

  // load the iguana algorithms library
  gSystem->Load("libIguanaAlgorithms");

  // start the inclusive kinematics algorithm
  iguana::physics::InclusiveKinematics algo;
  algo.Start();

  // run the inclusive kinematics action function for a scattered electron lepton momentum,
  // and print out the resulting inclusive kinematics
  auto key = algo.PrepareEvent(5032);
  auto result = algo.ComputeFromLepton(0.3, 0.3, 5.0, key);
  std::cout << "kinematics:"
    << "\n Q2 = " << result.Q2
    << "\n  x = " << result.x
    << "\n  W = " << result.W
    << std::endl;

  // stop the algorithm
  algo.Stop();
}
