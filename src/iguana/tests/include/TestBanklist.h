// test banklist behavior

#include <hipo4/reader.h>
#include <iguana/algorithms/AlgorithmSequence.h>

inline int TestBanklist(
    std::string data_file,
    bool verbose)
{

  if(data_file == "") {
    fmt::print(stderr, "ERROR: need a data file for command 'banklist'\n");
    return 1;
  }

  // set the concurrency model to single-threaded, for optimal performance
  iguana::GlobalConcurrencyModel = "single";

  // open the HIPO file
  hipo::reader reader(data_file.c_str());
  auto banks = reader.getBanks({
      "REC::Particle",
      "RUN::config",
      "REC::Track",
      "REC::Calorimeter",
      "REC::Scintillator",
  });

  // define the algorithm sequence
  iguana::AlgorithmSequence seq;
  seq.Add("clas12::SectorFinder", "creator_1a");
  seq.Add("physics::InclusiveKinematics", "creator_2");
  seq.Add("clas12::SectorFinder", "creator_1b");
  seq.PrintSequence();

  // start the sequence
  seq.Start(banks);

  // get names and indices of created banks
  auto bank_1a_index = seq.GetBankIndex(banks, seq.GetCreatedBankName("creator_1a"), "creator_1a");
  auto bank_1b_index = seq.GetBankIndex(banks, seq.GetCreatedBankName("creator_1b"), "creator_1b");
  auto bank_2_index  = seq.GetBankIndex(banks, seq.GetCreatedBankName("creator_2"), "creator_2");

  /////////////////////////
  // TODO:
  // assert bank indices, variant numbers, etc.
  /////////////////////////

  // stop the algorithm
  seq.Stop();
  return 0;
}
