// multithreaded test of an iguana algorithm

#include <hipo4/reader.h>
#include <iguana/algorithms/AlgorithmSequence.h>

inline int TestMultithreading(
    std::string command,
    std::string algo_name,
    std::vector<std::string> prerequisite_algos,
    std::vector<std::string> bank_names,
    std::string data_file,
    int num_events,
    int num_threads,
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

  // start the stream
  hipo::readerstream stream;
  stream.open(data_file.c_str());

  // define the worker function
  auto ftn = [&stream, &algo_name, &prerequisite_algos, &bank_names, &verbose](int order) {

    // frame with 128 events
    std::vector<hipo::event> events;
    for(int i = 0; i < 128; i++)
      events.push_back(hipo::event());

    // bank list
    hipo::banklist banks;
    for(auto const& bank_name : bank_names)
      banks.push_back(hipo::bank(stream.dictionary().getSchema(bank_name.c_str()),48));

    // define the algorithm
    iguana::AlgorithmSequence seq;
    for(auto const& prerequisite_algo : prerequisite_algos)
      seq.Add(prerequisite_algo);
    seq.Add(algo_name);
    seq.SetName("TEST thread " + std::to_string(order));
    seq.PrintSequence();
    seq.SetOption(algo_name, "log", verbose ? "info" : "info"); // FIXME

    // start the algorithm
    seq.Start(banks);

    // event frame loop
    int nProcessed = 0;
    while(true) {
      stream.pull(events);
      int nNonEmpty = 0;
      for(auto& event : events) {
        if(event.getSize() > 16) {
          nNonEmpty++;
          nProcessed++;
        }
        event.read(banks.front());
        if(banks.front().getRows() > 0) {
          seq.Run(banks);
        }
      }
      if(nNonEmpty == 0)
        break;
    }

    // stop the algorithm
    seq.Stop();

    return nProcessed;
  };

  // run
  stream.run(ftn, num_threads);
  return 0;
}
