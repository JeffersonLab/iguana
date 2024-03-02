// test an iguana algorithm validator

#include <hipo4/reader.h>
#include <iguana/algorithms/Validator.h>

inline int TestValidator(
    std::string algo_name,
    std::vector<std::string> bank_names,
    std::string data_file,
    int num_events,
    std::string output_dir,
    bool verbose)
{

  // check arguments
  if(algo_name == "" || bank_names.empty()) {
    fmt::print(stderr, "ERROR: need algorithm name and banks\n");
    return 1;
  }
  if(data_file == "") {
    fmt::print(stderr, "ERROR: need a data file for command 'validator'\n");
    return 1;
  }

  // open the HIPO file
  hipo::reader reader(data_file.c_str());
  auto banks = reader.getBanks(bank_names);

  // define the validator
  auto vdor = iguana::ValidatorFactory::Create(algo_name);
  vdor->SetOutputDirectory(output_dir);

  // event loop
  vdor->Start(banks);
  int it_ev = 0;
  while(reader.next(banks) && (num_events == 0 || it_ev++ < num_events)) {
    if(it_ev % 10000 == 0)
      fmt::print("Read {} events\n", it_ev);
    vdor->Run(banks);
  }
  vdor->Stop();
  return 0;
}
