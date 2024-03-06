#include <getopt.h>

#include "TestAlgorithm.h"
#include "TestConfig.h"
#include "TestValidator.h"

int main(int argc, char** argv)
{
  // user parameters
  std::string command    = "";
  std::string data_file  = "";
  int num_events         = 10;
  std::string algo_name  = "";
  int test_num           = 0;
  std::string output_dir = "";
  bool verbose           = false;
  std::vector<std::string> bank_names;

  // get the command
  auto UsageCommands = [&](int exit_code)
  {
    fmt::print(stderr, "\nUSAGE: {} [COMMAND] [OPTIONS]...\n", argv[0]);
    fmt::print("\n  COMMANDS:\n\n");
    fmt::print("    {:<20} {}\n", "algorithm", "call `Run` on an algorithm");
    fmt::print("    {:<20} {}\n", "validator", "run an algorithm's validator");
    fmt::print("    {:<20} {}\n", "unit", "call `Test` on an algorithm, for unit tests");
    fmt::print("    {:<20} {}\n", "config", "test config file parsing");
    fmt::print("\n  OPTIONS:\n\n");
    fmt::print("    Each command has its own set of OPTIONS; either provide no OPTIONS\n");
    fmt::print("    or use the --help option for more usage information about a specific command\n");
    fmt::print("\n");
    return exit_code;
  };
  if(argc <= 1)
    return UsageCommands(2);
  command = std::string(argv[1]);
  if(command == "--help" || command == "-h")
    return UsageCommands(2);
  // omit the command, for getopt
  argv++;
  argc--;

  // usage options
  auto UsageOptions = [&](int exit_code)
  {
    std::unordered_map<std::string, std::function<void()>> print_option = {
        {"f", [&]()
         {
           fmt::print("    {:<20} {}\n", "-f FILE", "input data file");
         }},
        {"n", [&]()
         {
           fmt::print("    {:<20} {}\n", "-n NUM_EVENTS", "number of events from the data file");
           fmt::print("    {:<20} set to 0 to process ALL events\n", "");
           fmt::print("    {:<20} default: {}\n", "", num_events);
         }},

        {"a-algo", [&]()
         {
           fmt::print("    {:<20} {}\n", "-a ALGORITHM", "the name of the algorithm");
         }},
        {"a-vdor", [&]()
         {
           fmt::print("    {:<20} {}\n", "-a VALIDATOR", "the name of the validator");
         }},
        {"b", [&]()
         {
           fmt::print("    {:<20} {}\n", "-b BANKS", "add a single bank to process");
           fmt::print("    {:<20} you may add as many banks as you need (-b BANK1 -b BANK2 ...)\n", "");
           fmt::print("    {:<20} default: if you do not add any banks, ALL of them will be used\n", "");
         }},

        {"t", [&]()
         {
           fmt::print("    {:<20} {}\n", "-t TESTNUM", "test number");
         }},
        {"o", [&]()
         {
           fmt::print("    {:<20} {}\n", "-o OUTPUT_DIR", "if specified, validators will write to this directory");
         }},
        {"v", [&]()
         {
           fmt::print("    {:<20} {}\n", "-v", "increase verbosity");
         }}};
    std::vector<std::string> available_options;
    if(command == "algorithm" || command == "unit") {
      available_options = {"f", "n", "a-algo", "b"};
    }
    else if(command == "validator") {
      available_options = {"f", "n", "a-vdor", "b", "o"};
    }
    else if(command == "config") {
      available_options = {"t"};
    }
    else {
      fmt::print(stderr, "ERROR: unknown command '{}'\n", command);
      return 1;
    }
    available_options.push_back("v");
    fmt::print(stderr, "\nUSAGE: {} {} [OPTIONS]...\n", argv[0], command);
    fmt::print("\n  OPTIONS:\n\n");
    for(auto available_opt : available_options) {
      print_option.at(available_opt)();
      fmt::print("\n");
    }
    return exit_code;
  };
  if(argc <= 2)
    return UsageOptions(2);
  auto first_option = std::string(argv[2]);
  if(first_option == "--help" || first_option == "-h")
    return UsageOptions(2);

  // parse option arguments
  int opt;
  while((opt = getopt(argc, argv, "hf:n:a:b:t:o:v|")) != -1) {
    switch(opt) {
    case 'h':
      return UsageOptions(2);
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
    case 'o':
      output_dir = std::string(optarg);
      break;
    case 'v':
      verbose = true;
      break;
    default:
      return UsageOptions(1);
    }
  }

  // list of ALL banks needed by the algorithms and validators; we need all of them here,
  // so that the caller does not have to specifiy the banks
  const std::vector<std::string> all_bank_names = {
      "RUN::config",
      "REC::Particle"};
  if(bank_names.empty())
    bank_names = all_bank_names;

  fmt::print("TEST IGUANA:\n");
  fmt::print("  {:>20} = {}\n", "command", command);
  fmt::print("  {:>20} = {}\n", "data_file", data_file);
  fmt::print("  {:>20} = {}\n", "num_events", num_events);
  fmt::print("  {:>20} = {}\n", "algo_name", algo_name);
  fmt::print("  {:>20} = {}\n", "banks", fmt::join(bank_names, ", "));
  fmt::print("  {:>20} = {}\n", "test_num", test_num);
  fmt::print("  {:>20} = {}\n", "output_dir", output_dir);
  fmt::print("\n");

  // run test
  if(command == "algorithm" || command == "unit") {
    return TestAlgorithm(command, algo_name, bank_names, data_file, num_events, verbose);
  }
  else if(command == "validator") {
    return TestValidator(algo_name, bank_names, data_file, num_events, output_dir, verbose);
  }
  else if(command == "config") {
    return TestConfig(test_num, verbose);
  }
  else {
    fmt::print(stderr, "ERROR: unknown command '{}'\n", command);
    return 1;
  }
  return 0;
}
