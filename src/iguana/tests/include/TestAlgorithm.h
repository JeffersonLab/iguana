// test an iguana algorithm

#include <hipo4/reader.h>
#include <iguana/algorithms/AlgorithmSequence.h>

inline int TestAlgorithm(
    std::string command,
    std::string algo_name,
    std::vector<std::string> prerequisite_algos,
    std::vector<std::string> bank_names,
    std::string data_file,
    int num_events,
    bool verbose)
{

  // check arguments
  if(algo_name == "" || bank_names.empty()) {
    fmt::print(stderr, "ERROR: need algorithm name and banks\n");
    return 1;
  }
  if(command == "algorithm" && data_file == "") {
    fmt::print(stderr, "ERROR: need a data file for command 'algorithm'\n");
    return 1;
  }

  // set the concurrency model to single-threaded, for optimal performance
  iguana::GlobalConcurrencyModel = "single";

  // open the HIPO file; we use 2 readers, one for 'before' (i.e., not passed through iguana), and one for 'after'
  // (passed through iguana), so we may compare them
  hipo::reader reader_before(data_file.c_str()); // NOTE: not copy-constructable, so make two separate readers
  hipo::reader reader_after(data_file.c_str());
  auto banks_before = reader_before.getBanks(bank_names);
  auto banks_after  = reader_after.getBanks(bank_names);

  // define the algorithm
  iguana::AlgorithmSequence seq;
  for(auto const& prerequisite_algo : prerequisite_algos)
    seq.Add(prerequisite_algo);
  seq.Add(algo_name);
  seq.SetName("TEST");
  seq.PrintSequence();
  seq.SetOption(algo_name, "log", verbose ? "trace" : "info");

  // start the algorithm
  seq.Start(banks_after);

  // event loop
  int it_ev = 0;
  while(reader_after.next(banks_after) && (num_events == 0 || it_ev++ < num_events)) {
    // iterate the 'before' reader too
    reader_before.next(banks_before);
    // run the algorithm
    if(command == "algorithm")
      seq.Run(banks_after);
    else if(command == "unit") {
      fmt::print(stderr, "ERROR: unit tests are not yet implemented (TODO)\n");
      return 1;
    }
    else {
      fmt::print(stderr, "ERROR: unknown command '{}'\n", command);
      return 1;
    }
    // print the banks, before and after
    if(verbose) {
      for(decltype(bank_names)::size_type it_bank = 0; it_bank < bank_names.size(); it_bank++) {
        fmt::print("{:=^70}\n", fmt::format(" BEFORE: {} ", bank_names.at(it_bank)));
        banks_before.at(it_bank).show();
        fmt::print("{:=^70}\n", fmt::format(" AFTER: {} ", bank_names.at(it_bank)));
        banks_after.at(it_bank).show();
        fmt::print("\n");
      }
    }
  }

  // stop the algorithm
  seq.Stop();
  return 0;
}
