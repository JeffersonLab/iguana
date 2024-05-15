#include <TSystem.h>

void iguanaROOT() {
  gSystem->Load("libIguanaAlgorithms.so");
  gInterpreter->AddIncludePath("-I/home/dilks/j/install/include"); // HIPO
  gInterpreter->AddIncludePath("-I/home/dilks/j/iguana/iguana/include"); // IGUANA
}
