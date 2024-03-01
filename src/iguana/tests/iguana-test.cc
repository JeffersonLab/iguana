#include <getopt.h>

#include "TestAlgorithm.h"
#include "TestConfig.h"

int main(int argc, char** argv)
{
  // user parameters
  std::string command   = "";
  std::string data_file = "";
  int num_events        = 10;
  std::string algo_name = "";
  int test_num          = 0;
  bool verbose          = false;
  std::vector<std::string> bank_names;

  // usage
  auto Usage = [&](int exit_code)
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
    return TestAlgorithm(command, algo_name, bank_names, data_file, num_events, verbose);
  }
  else if(command == "config") {
    return TestConfig(test_num, verbose);
  }
  else {
    fmt::print(stderr, "ERROR: need at least a command\n");
    return Usage(1);
  }
  return 0;
}
