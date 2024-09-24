// multithreaded test of an iguana algorithm

#include <hipo4/reader.h>
#include <iguana/algorithms/AlgorithmSequence.h>
#include <iguana/services/GlobalParam.h>

inline int TestMultithreading(
    std::string command,
    std::string algo_name,
    std::vector<std::string> prerequisite_algos,
    std::vector<std::string> bank_names,
    std::string data_file,
    int num_events,
    int num_threads,
    std::string concurrency_model,
    bool vary_run,
    bool verbose)
{

  iguana::Logger log("test", verbose ? iguana::Logger::Level::trace : iguana::Logger::Level::info);

  // check arguments
  if(algo_name.empty() || bank_names.empty()) {
    log.Error("need algorithm name and banks");
    return 1;
  }
  if(data_file.empty()) {
    log.Error("need a data file for command {:?}", command);
    return 1;
  }

  // set the concurrency model
  if(!concurrency_model.empty())
    iguana::GlobalConcurrencyModel = concurrency_model;

  // find the 'RUN::config' bank, if any
  std::optional<hipo::banklist::size_type> run_config_bank_idx{};
  if(vary_run) {
    for(hipo::banklist::size_type idx = 0; idx < bank_names.size(); idx++) {
      if(bank_names.at(idx) == "RUN::config") {
        run_config_bank_idx = idx;
        break;
      }
    }
  }

  // number of events per thread
  int const default_frame_size = 50;
  int num_events_per_thread = (int) std::round((double) num_events / num_threads);
  int num_events_per_frame  = num_events > 0 ? std::min(num_events_per_thread, default_frame_size) : default_frame_size;
  int num_frames_per_thread = num_events > 0 ? (int) std::ceil((double) num_events_per_thread / num_events_per_frame) : 0;
  int num_events_actual     = num_events_per_frame * num_frames_per_thread * num_threads;
  log.Info("num_events_per_thread = {}", num_events_per_thread);
  log.Info("num_events_per_frame  = {}", num_events_per_frame );
  log.Info("num_frames_per_thread = {}", num_frames_per_thread);
  if(num_events > 0) {
    log.Info("=> will actually process num_events = {}", num_events_actual);
    if(num_events != num_events_actual)
      log.Warn("argument's num_events ({}) differs from the actual num_events that will be processed ({})",
          num_events, num_events_actual);
  } else {
    log.Info("=> will actually process num_events = ALL OF THEM");
  }

  // start the stream
  hipo::readerstream stream;
  stream.open(data_file.c_str());

  // define the worker function
  auto ftn = [
    &stream,
    algo_name,
    prerequisite_algos,
    bank_names,
    vary_run,
    verbose,
    num_events_per_thread,
    num_events_per_frame,
    run_config_bank_idx
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
    seq.SetOption(algo_name, "log", verbose ? "trace" : "info");

    // start the algorithm
    seq.Start(banks);

    // loop over frames
    int nProcessed = 0;
    while(nProcessed < num_events_per_thread || num_events_per_thread == 0) {
      stream.pull(events);

      // loop over events in this frame
      int nNonEmpty = 0;
      for(auto& event : events) {
        if(event.getSize() > 16) {
          nNonEmpty++;
          nProcessed++;

          // read the banks
          for(auto& bank : banks)
            event.read(bank);

          // occasionally vary the run number (if the user wants to)
          if(vary_run && run_config_bank_idx.has_value()) {
            if(std::rand() % 10 == 0) {
              auto runnum = banks[run_config_bank_idx.value()].getInt("run", 0);
              runnum += (std::rand() % 2 == 0) ? 1 : -1; // randomly increase or decrease the runnum by 1
              banks[run_config_bank_idx.value()].putInt("run", 0, runnum);
            }
            else if(std::rand() % 10 == 1) {
              banks[run_config_bank_idx.value()].putInt("run", 0, 1); // set the runnum to '1'
            }
          }

          // run the iguana algorithm
          seq.Run(banks, order);
        }
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
