#include <cassert>
#include <getopt.h>
#include <hipo4/reader.h>
#include <iguana/algorithms/AlgorithmSequence.h>

bool verbose          = false;
std::function<int(int)> Usage;

// test an iguana algorithm
// ------------------------------------------------------------------------------
int TestAlgorithm(
    std::string command,
    std::string algo_name,
    std::vector<std::string> bank_names,
    std::string data_file,
    int num_events)
{

  // check arguments
  if(algo_name == "" || bank_names.empty()) {
    fmt::print(stderr, "ERROR: need algorithm name and banks\n");
    return Usage(1);
  }
  if(command == "algorithm" && data_file == "") {
    fmt::print(stderr, "ERROR: need a data file for command 'algorithm'\n");
    return Usage(1);
  }

  // open the HIPO file; we use 2 readers, one for 'before' (i.e., not passed through iguana), and one for 'after'
  // (passed through iguana), so we may compare them
  hipo::reader reader_before(data_file.c_str()); // NOTE: not copy-constructable, so make two separate readers
  hipo::reader reader_after(data_file.c_str());
  auto banks_before = reader_before.getBanks(bank_names);
  auto banks_after  = reader_after.getBanks(bank_names);

  // define the algorithm
  iguana::AlgorithmSequence seq;
  seq.Add(algo_name);
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

// test algorithm configuration
// ------------------------------------------------------------------------------
int TestConfig(int test_num) {
  if(test_num == 0) {
    fmt::print(stderr, "ERROR: need a test number\n");
    return Usage(1);
  }
  auto algo = iguana::AlgorithmFactory::Create("example::ExampleAlgorithm");
  algo->SetOption("config_dir", "config/test");
  algo->SetOption("config_file", fmt::format("test_{}.yaml", test_num));
  algo->Start();

  switch(test_num) {

    case 1:
    {
      // test `GetOptionScalar`
      assert((algo->GetOptionScalar<int>("scalar_int") == 1));
      assert((algo->GetOptionScalar<double>("scalar_double") == 2.5));
      assert((algo->GetOptionScalar<std::string>("scalar_string") == "lizard"));
      // test `GetOptionVector`
      assert((algo->GetOptionVector<int>("vector_int") == std::vector<int>{1, 2, 3}));
      assert((algo->GetOptionVector<double>("vector_double") == std::vector<double>{1.5, 2.5}));
      assert((algo->GetOptionVector<std::string>("vector_string") == std::vector<std::string>{"bat", "chameleon", "spider"}));
      // test `GetOptionSet`
      auto s = algo->GetOptionSet<std::string>("vector_string");
      assert((s.find("spider") != s.end()));
      assert((s.find("bee") == s.end()));
      // test empty access
      try {
        algo->GetOptionScalar<int>("scalar_empty");
        throw std::runtime_error("accessing 'scalar_empty' did not throw exception");
      }
      catch(const std::exception& ex) {
        // do nothing, since we expect an exception
      }
      try {
        algo->GetOptionVector<int>("vector_empty");
        throw std::runtime_error("accessing 'vector_empty' did not throw exception");
      }
      catch(const std::exception& ex) {
        // do nothing, since we expect an exception
      }
      break;
    }

    default:
      fmt::print(stderr, "ERROR: unknown test number '{}'", test_num);
      return 1;
  }
  return 0;
}

// main
// ------------------------------------------------------------------------------
int main(int argc, char** argv)
{
  // user parameters
  std::string command   = "";
  std::string data_file = "";
  int num_events        = 10;
  std::string algo_name = "";
  int test_num          = 0;
  std::vector<std::string> bank_names;

  // usage
  Usage = [&](int exit_code)
  {
    fmt::print(stderr, "\nUSAGE: {} [OPTIONS]...\n", argv[0]);
    fmt::print("\n");
    fmt::print("  OPTIONS:\n");
    fmt::print("\n");
    fmt::print("    {:<20} {}\n\n", "-c COMMAND", "which test command to use:");
    fmt::print("    {:<20} {:<15} {}\n", "", "algorithm", "call `Run` on an algorithm");
    fmt::print("    {:<20} {:<15} {}\n", "", "unit", "call `Test` on an algorithm, for unit tests");
    fmt::print("    {:<20} {:<15} {}\n", "", "config", "test config file parsing");
    fmt::print("\n");
    fmt::print("    {:<20} {}\n", "-f FILE", "input data file, for when COMMAND==algorithm");
    fmt::print("\n");
    fmt::print("    {:<20} {}\n", "-n NUM_EVENTS", "number of events from the data file");
    fmt::print("    {:<20} default: {}\n", "", num_events);
    fmt::print("\n");
    fmt::print("    {:<20} {}\n", "-a ALGORITHM", "the name of the algorithm");
    fmt::print("\n");
    fmt::print("    {:<20} {}\n", "-b BANKS", "add a bank to process");
    fmt::print("\n");
    fmt::print("    {:<20} {}\n", "-t TESTNUM", "test number, for commands that need one");
    fmt::print("\n");
    fmt::print("    {:<20} {}\n", "-v", "increase verbosity");
    fmt::print("\n");
    return exit_code;
  };
  if(argc == 1)
    return Usage(2);

  // parse arguments
  int opt;
  while((opt = getopt(argc, argv, "c:f:n:a:b:t:v|")) != -1) {
    switch(opt) {
    case 'c':
      command = std::string(optarg);
      break;
    case 'f':
      data_file = std::string(optarg);
      break;
    case 'n':
      num_events = std::stoi(optarg);
      break;
    case 'a':
      algo_name = std::string(optarg);
      break;
    case 'b':
      bank_names.push_back(std::string(optarg));
      break;
    case 't':
      test_num = std::stoi(optarg);
      break;
    case 'v':
      verbose = true;
      break;
    default:
      fmt::print(stderr, "ERROR: unkwnown option '{}'", opt);
      return Usage(1);
    }
  }
  if(verbose) {
    fmt::print("TEST IGUANA:\n");
    fmt::print("  {:>20} = {}\n", "command", command);
    fmt::print("  {:>20} = {}\n", "data_file", data_file);
    fmt::print("  {:>20} = {}\n", "num_events", num_events);
    fmt::print("  {:>20} = {}\n", "algo_name", algo_name);
    fmt::print("  {:>20} = {}\n", "banks", fmt::join(bank_names, ", "));
    fmt::print("  {:>20} = {}\n", "test_num", test_num);
    fmt::print("\n");
  }

  // run test
  if(command == "algorithm" || command == "unit") {
    return TestAlgorithm(command, algo_name, bank_names, data_file, num_events);
  }
  else if(command == "config") {
    return TestConfig(test_num);
  }
  else {
    fmt::print(stderr, "ERROR: need at least a command\n");
    return Usage(1);
  }
  return 0;
}
