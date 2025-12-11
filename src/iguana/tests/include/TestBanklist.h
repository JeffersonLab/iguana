// test banklist behavior

#include <cassert>
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

  // input bank names
  std::vector<std::string> input_bank_names = {
      "REC::Particle",
      "RUN::config",
      "REC::Track",
      "REC::Calorimeter",
      "REC::Scintillator",
  };

  // open the HIPO file
  hipo::reader reader(data_file.c_str());
  auto banks = reader.getBanks(input_bank_names);

  // define the algorithm sequence
  std::vector<std::string> algo_names = {
      "creator_1a",
      "creator_2",
      "creator_1b",
  };
  iguana::AlgorithmSequence seq;
  seq.Add("clas12::SectorFinder", "creator_1a");
  seq.Add("clas12::CalorimeterLinker", "creator_2");
  seq.Add("clas12::SectorFinder", "creator_1b");

  // print the sequence
  seq.PrintSequence();

  // start the sequence
  seq.Start(banks);

  // print the banklist
  fmt::println("BANKLIST:");
  for(decltype(banks)::size_type i = 0; i < banks.size(); i++) {
    auto& bank = banks.at(i);
    fmt::println(" - {}: {}", i, bank.getSchema().getName());
  }

  // assert variants
  assert((seq.Get("creator_1a")->GetCreatedBankVariant() == 0));
  assert((seq.Get("creator_1b")->GetCreatedBankVariant() == 1));
  assert((seq.Get("creator_2")->GetCreatedBankVariant() == 0));

  // assert return values of `iguana::tools::GetBankIndex`
  //// input banks
  for(decltype(input_bank_names)::size_type i = 0; i < input_bank_names.size(); i++) {
    auto const& bank_name = input_bank_names.at(i);
    auto const idx        = iguana::tools::GetBankIndex(banks, bank_name);
    assert((idx == i));
  }
  //// output banks
  for(decltype(algo_names)::size_type i = 0; i < algo_names.size(); i++) {
    auto const& algo_name = algo_names.at(i);
    auto const idx        = iguana::tools::GetBankIndex(banks, seq.GetCreatedBankName(algo_name), seq.Get(algo_name)->GetCreatedBankVariant());
    assert((idx == input_bank_names.size() + i));
  }

  // assert equivalence to `Algorithm::GetBankIndex` and `AlgorithmSequence::GetBankIndex`
  for(auto const& algo_name : algo_names) {
    //// input banks
    for(auto const& bank_name : input_bank_names) {
      auto const idx = iguana::tools::GetBankIndex(banks, bank_name);
      assert((idx == seq.Get(algo_name)->GetBankIndex(banks, bank_name)));
      assert((idx == seq.GetBankIndex(banks, bank_name, algo_name)));
    }
    //// output banks
    auto const idx = iguana::tools::GetBankIndex(banks, seq.GetCreatedBankName(algo_name), seq.Get(algo_name)->GetCreatedBankVariant());
    assert((idx == seq.Get(algo_name)->GetBankIndex(banks, seq.GetCreatedBankName(algo_name))));
    assert((idx == seq.GetBankIndex(banks, seq.GetCreatedBankName(algo_name), algo_name)));
  }

  // assert equivalence to `Algorithm::GetCreatedBankIndex` and `AlgorithmSequence::GetCreatedBankIndex`
  for(auto const& algo_name : algo_names) {
    auto const idx = iguana::tools::GetBankIndex(banks, seq.GetCreatedBankName(algo_name), seq.Get(algo_name)->GetCreatedBankVariant());
    assert((idx == seq.Get(algo_name)->GetCreatedBankIndex(banks)));
    assert((idx == seq.GetCreatedBankIndex(banks, algo_name)));
  }

  // stop the algorithm
  seq.Stop();
  return 0;
}
