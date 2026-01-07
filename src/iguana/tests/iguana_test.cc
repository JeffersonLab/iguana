#include <getopt.h>

#include "TestAlgorithm.h"
#include "TestBanklist.h"
#include "TestConfig.h"
#include "TestLogger.h"
#include "TestMultithreading.h"
#include "TestValidator.h"
#include <iguana/services/Tools.h>

int main(int argc, char** argv)
{
  // user parameters
  std::string command           = "";
  std::string data_file         = "";
  int num_events                = 10;
  std::string algo_name         = "";
  int test_num                  = 0;
  int num_threads               = 0;
  std::string concurrency_model = "";
  bool vary_run                 = false;
  std::string output_dir        = "";
  int verbosity                 = 0;
  std::vector<std::string> bank_names;
  std::vector<std::string> prerequisite_algos;

  // get the command
  auto exe           = std::string(argv[0]);
  auto UsageCommands = [&](int exit_code) {
    fmt::print("\nUSAGE: {} [COMMAND] [OPTIONS]...\n", exe);
    fmt::print("\n  COMMANDS:\n\n");
    fmt::print("    {:<20} {}\n", "algorithm", "call `Run` on an algorithm");
    fmt::print("    {:<20} {}\n", "multithreading", "call `Run` on an algorithm, multithreaded");
    fmt::print("    {:<20} {}\n", "validator", "run an algorithm's validator");
    fmt::print("    {:<20} {}\n", "unit", "call `Test` on an algorithm, for unit tests");
    fmt::print("    {:<20} {}\n", "config", "test config file parsing");
    fmt::print("    {:<20} {}\n", "logger", "test Logger");
    fmt::print("    {:<20} {}\n", "banklist", "test hipo::banklist");
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
    return UsageCommands(0);
  // omit the command, for getopt
  argv++;
  argc--;

  // usage options
  // clang-format off
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
        {"p", [&]()
         {
           fmt::print("    {:<20} {}\n", "-p PREREQUISITE_ALGOS", "add a prerequisite algorithm");
           fmt::print("    {:<20} these are the algorithms needed upstream of ALGORITHM\n", "");
           fmt::print("    {:<20} this option is repeatable\n", "");
           fmt::print("    {:<20} default: no prerequisites\n", "");
         }},
        {"t", [&]()
         {
           fmt::print("    {:<20} {}\n", "-t TESTNUM", "test number");
         }},
        {"j", [&]()
         {
           fmt::print("    {:<20} {}\n", "-j NUM_THREADS", "number of threads to run");
           fmt::print("    {:<20} - if = 0: run with `std::thread::hardware_concurrency()` threads\n", "");
           fmt::print("    {:<20} - if > 0: run with NUM_THREADS threads\n", "");
           fmt::print("    {:<20} default: {}\n", "", num_threads);
         }},
        {"m", [&]()
         {
           fmt::print("    {:<20} {}\n", "-m CONCURRENCY_MODEL", "concurrency model");
           fmt::print("    {:<20} 'memoize' is currently the only option\n", "");
         }},
        {"V", [&]()
         {
           fmt::print("    {:<20} {}\n", "-V", "randomly vary the run number");
           fmt::print("    {:<20} this is for testing run-dependent configuration thread safety\n", "");
         }},
        {"o", [&]()
         {
           fmt::print("    {:<20} {}\n", "-o OUTPUT_DIR", fmt::format("if specified, {} output will write to this directory;", command));
           fmt::print("    {:<20} if not specified, output will not be written\n", "");
         }},
        {"v", [&]()
         {
           fmt::print("    {:<20} {}\n", "-v", "increase verbosity by one level;");
           fmt::print("    {:<20} repeated uses increase verbosity more\n", "");
         }}};
    std::map<std::string, std::vector<std::string>> available_options = {
      {"algorithm",      {"f", "n", "a-algo", "b", "p"}},
      {"unit",           {"f", "n", "a-algo", "b", "p"}},
      {"multithreading", {"f", "n", "a-algo", "b", "p", "j", "m", "V"}},
      {"validator",      {"f", "n", "a-vdor", "b", "o"}},
      {"config",         {"t"}},
      {"logger",         {}},
      {"banklist",       {"f"}}
    };
    for(auto& it : available_options)
      it.second.push_back("v");
    if(auto it{available_options.find(command)}; it != available_options.end()) {
      fmt::print("\nUSAGE: {} {} [OPTIONS]...\n", exe, command);
      fmt::print("\n  OPTIONS:\n\n");
      for(auto available_opt : it->second) {
        print_option.at(available_opt)();
        fmt::print("\n");
      }
      return exit_code;
    }
    else {
      fmt::print(stderr, "ERROR: unknown command '{}'\n", command);
      return 1;
    }
  };
  // clang-format on

  auto first_option = argc >= 2 ? std::string(argv[1]) : "";
  if(first_option == "--help" || first_option == "-h")
    return UsageOptions(0);
  if(argc <= 2 && command != "logger")
    return UsageOptions(2);

  // parse option arguments
  int opt;
  while((opt = getopt(argc, argv, "hf:n:a:b:p:t:j:m:Vo:v|")) != -1) {
    switch(opt) {
    case 'h':
      return UsageOptions(0);
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
    case 'p':
      prerequisite_algos.push_back(std::string(optarg));
      break;
    case 't':
      test_num = std::stoi(optarg);
      break;
    case 'j':
      num_threads = std::stoi(optarg);
      if(num_threads == 0)
        num_threads = std::thread::hardware_concurrency();
      break;
    case 'm':
      concurrency_model = std::string(optarg);
      break;
    case 'V':
      vary_run = true;
      break;
    case 'o':
      output_dir = std::string(optarg);
      break;
    case 'v':
      verbosity++;
      break;
    default:
      return UsageOptions(2);
    }
  }

  // list of ALL banks needed by the algorithms and validators; we need all of them here,
  // so that the caller does not have to specifiy the banks
  std::vector<std::string> const all_bank_names = {
      "RUN::config",
      "REC::Particle",
      "REC::Calorimeter",
      "REC::Track",
      "REC::Scintillator",
      "REC::Traj"};
  if(bank_names.empty())
    bank_names = all_bank_names;

  fmt::print("TEST IGUANA:\n");
  fmt::print("  {:>20} = {}\n", "command", command);
  fmt::print("  {:>20} = {}\n", "data_file", data_file);
  fmt::print("  {:>20} = {}\n", "num_events", num_events);
  fmt::print("  {:>20} = {}\n", "algo_name", algo_name);
  fmt::print("  {:>20} = {}\n", "banks", fmt::join(bank_names, ", "));
  fmt::print("  {:>20} = {}\n", "prerequisite_algos", fmt::join(prerequisite_algos, ", "));
  fmt::print("  {:>20} = {}\n", "test_num", test_num);
  fmt::print("  {:>20} = {}\n", "num_threads", num_threads);
  fmt::print("  {:>20} = {}\n", "concurrency_model", concurrency_model);
  fmt::print("  {:>20} = {}\n", "vary_run", vary_run);
  fmt::print("  {:>20} = {}\n", "output_dir", output_dir);
  fmt::print("\n");

  // expand `~` in paths
  data_file  = iguana::tools::ExpandTilde(data_file);
  output_dir = iguana::tools::ExpandTilde(output_dir);

  // set log level
  std::string log_level;
  switch(verbosity) {
  case 0:
    log_level = "info";
    break;
  case 1:
    log_level = "debug";
    break;
  case 2:
    log_level = "trace";
    break;
  default:
    fmt::println(stderr, "WARNING: no higher verbosity levels are available for `iguana_test`");
    log_level = "trace";
    break;
  }

  // run test
  if(command == "algorithm" || command == "unit")
    return TestAlgorithm(command, algo_name, prerequisite_algos, bank_names, data_file, num_events, log_level);
  if(command == "multithreading")
    return TestMultithreading(command, algo_name, prerequisite_algos, bank_names, data_file, num_events, num_threads, concurrency_model, vary_run, log_level);
  else if(command == "validator")
    return TestValidator(algo_name, bank_names, data_file, num_events, output_dir, log_level);
  else if(command == "config")
    return TestConfig(test_num, log_level);
  else if(command == "logger")
    return TestLogger();
  else if(command == "banklist")
    return TestBanklist(data_file);
  else {
    fmt::print(stderr, "ERROR: unknown command '{}'\n", command);
    return 1;
  }
  return 0;
}
