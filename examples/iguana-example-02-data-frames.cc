/// @begin_doc_example
/// @file iguana-example-02-data-frames.cc
/// @brief Example using HIPO data frames with Iguana. Requires ROOT.
/// @par Usage
/// ```bash
/// iguana-example-02-data-frames [HIPO_FILE] [NUM_EVENTS]
///
///   HIPO_FILE   the HIPO file to analyze
///
///   NUM_EVENTS  the number of events to analyze;
///               set to zero to analyze all events
/// ```
/// @end_doc_example

#include <hipo4/RHipoDS.hxx>
#include <iguana/algorithms/clas12/EventBuilderFilter.h>

/// main function
int main(int argc, char** argv)
{

  // parse arguments
  int argi               = 1;
  char const* inFileName = argc > argi ? argv[argi++] : "data.hipo";
  // int const numEvents    = argc > argi ? std::stoi(argv[argi++]) : 1;

  // iguana algorithms
  iguana::clas12::EventBuilderFilter algo_eventbuilder_filter;
  algo_eventbuilder_filter.SetOption<std::vector<int>>("pids", {11, 211, -211});
  algo_eventbuilder_filter.Start();

  // enable multi-threading
  ROOT::EnableImplicitMT();

  // open the HIPO file
  auto frame_init = MakeHipoDataFrame(inFileName);

  // run algorithms
  // auto frame_filtered = frame_init.Define(
  //     "REC::Particle::event_builder_filter",
  //     [&algo_eventbuilder_filter](auto& pids) { return algo_eventbuilder_filter.Filter(pids); },
  //     {"REC::Particle::pid"});

  return 0;
}
