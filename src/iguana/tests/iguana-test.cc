#include <iguana/algorithms/AlgorithmSequence.h>
#include <hipo4/reader.h>

int main(int argc, char **argv) {

  // parse arguments
  if(argc-1 < 3) {
    fmt::print(stderr, "USAGE: {} [data_file] [num_events] [algorithm_name] [banks]...\n", argv[0]);
    return 2;
  }
  const std::string data_file  = std::string(argv[1]);
  const int         num_events = std::stoi(argv[2]);
  const std::string algo_name  = std::string(argv[3]);
  std::vector<std::string> bank_names;
  for(int i=4; i<argc; i++)
    bank_names.push_back(std::string(argv[i]));
  fmt::print("TEST IGUANA:\n");
  fmt::print("  {:>20} = {}\n", "data_file", data_file);
  fmt::print("  {:>20} = {}\n", "num_events", num_events);
  fmt::print("  {:>20} = {}\n", "algo_name", algo_name);
  fmt::print("  {:>20} = {}\n", "banks", fmt::join(bank_names,", "));
  fmt::print("\n");

  // open the HIPO file; we use 2 readers, one for 'before' (i.e., not passed through iguana), and one for 'after'
  // (passed through iguana), so we may compare them
  hipo::reader reader_before(data_file.c_str()); // NOTE: not copy-constructable, so make two separate readers
  hipo::reader reader_after(data_file.c_str());
  auto banks_before = reader_before.getBanks(bank_names);
  auto banks_after  = reader_after.getBanks(bank_names);

  // define the algorithm
  iguana::AlgorithmSequence seq;
  seq.Add(algo_name);
  seq.SetOption(algo_name, "log", "debug");

  // start the algorithm
  seq.Start(banks_after);

  // event loop
  int it_ev = 0;
  while(reader_after.next(banks_after) && (num_events==0 || it_ev++ < num_events)) {
    // iterate the 'before' reader too
    reader_before.next(banks_before);
    // run the algorithm
    seq.Run(banks_after);
    // print the banks, before and after
    for(decltype(bank_names)::size_type it_bank=0; it_bank < bank_names.size(); it_bank++) {
      fmt::print("{:=^70}\n", fmt::format(" BEFORE: {} ", bank_names.at(it_bank)));
      banks_before.at(it_bank).show();
      fmt::print("{:=^70}\n", fmt::format(" AFTER: {} ", bank_names.at(it_bank)));
      banks_after.at(it_bank).show();
    }
  }

  // stop the algorithm
  seq.Stop();
  return 0;
}
