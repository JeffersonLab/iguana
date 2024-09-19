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

  iguana::Logger log("test", iguana::Logger::Level::debug);

  // check arguments
  if(algo_name.empty() || bank_names.empty()) {
    log.Error("need algorithm name and banks");
    return 1;
  }
  if(data_file.empty()) {
    log.Error("need a data file for command {:?}", command);
    return 1;
  }

  // number of events per thread
  int num_events_per_thread = (int) std::round((double) num_events / num_threads);
  int num_events_per_frame  = std::min(num_events_per_thread, 50);
  int num_frames_per_thread = (int) std::ceil((double) num_events_per_thread / num_events_per_frame);
  int num_events_actual     = num_events_per_frame * num_frames_per_thread * num_threads;
  log.Debug("num_events_per_thread = {}", num_events_per_thread);
  log.Debug("num_events_per_frame  = {}", num_events_per_frame );
  log.Debug("num_frames_per_thread = {}", num_frames_per_thread);
  log.Debug("=> will actually process num_events = {}", num_events_actual);
  if(num_events != num_events_actual)
    log.Warn("argument's num_events ({}) differs from the actual num_events that will be processed ({})",
        num_events, num_events_actual);

  // start the stream
  hipo::readerstream stream;
  stream.open(data_file.c_str());

  // define the worker function
  auto ftn = [
    &stream,
    algo_name,
    prerequisite_algos,
    bank_names,
    verbose,
    num_events_per_thread,
    num_events_per_frame
  ](int order) {

    // fill a frame
    std::vector<hipo::event> events;
    for(int i = 0; i < num_events_per_frame; i++)
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
    while(nProcessed < num_events_per_thread) {
      stream.pull(events);
      int nNonEmpty = 0;
      for(auto& event : events) {
        if(event.getSize() > 16) {
          nNonEmpty++;
          nProcessed++;
        }
        for(auto& bank : banks)
          event.read(bank);
        seq.Run(banks);
      }
      if(nNonEmpty == 0)
        break;
    }

    // stop the algorithm
    seq.Stop();

    seq.GetLog()->Info("nProcessed = {}", nProcessed);
    return nProcessed;
  };

  // run
  stream.run(ftn, num_threads);
  return 0;
}
