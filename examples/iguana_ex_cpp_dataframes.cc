/// @begin_doc_example{cpp}
/// @file iguana_ex_cpp_dataframes.cc
/// @brief Example using HIPO data frames with Iguana. Requires ROOT.
/// @par Usage
/// ```bash
/// iguana_ex_cpp_dataframes [HIPO_FILE] [NUM_EVENTS] [INTERACTIVE_MODE]
///
///   HIPO_FILE         the HIPO file to analyze
///
///   NUM_EVENTS        the number of events to analyze;
///                     set to zero to analyze all events
///
///   INTERACTIVE_MODE  if 'true', plot will be drawn interactively,
///                     otherwise it will be saved to a PNG file
///                     (default = false)
/// ```
/// @end_doc_example

#include <hipo4/RHipoDS.hxx>
#include <iguana/algorithms/clas12/EventBuilderFilter/Algorithm.h>

#include <TApplication.h>
#include <TCanvas.h>

/// main function
int main(int argc, char** argv)
{

  // parse arguments
  char const* in_file         = argc > 1 ? argv[1] : "data.hipo";
  int const num_events        = argc > 2 ? std::stoi(argv[2]) : 100;
  bool const interactive_mode = argc > 3 ? std::string(argv[3]) == "true" : false;

  // iguana algorithms
  iguana::clas12::EventBuilderFilter algo_eventbuilder_filter;
  algo_eventbuilder_filter.SetOption<std::vector<int>>("pids", {11, 211, -211});
  algo_eventbuilder_filter.Start();

  // enable interactive mode
  auto app = interactive_mode ? new TApplication("app", &argc, argv) : nullptr;

  // enable multi-threading
  // ROOT::EnableImplicitMT();

  // open the HIPO file
  auto frame_init = MakeHipoDataFrame(in_file).Range(0, num_events);

  // print the column names
  fmt::print("DATAFRAME COLUMNS:\n");
  for(auto const& column_name : frame_init.GetColumnNames())
    fmt::print(" - {}\n", column_name);

  // run algorithms
  // FIXME: we want to be able to apply an Iguana Filter as an RDataFrame Filter to a set of columns;
  // this example chain filters only `REC::Particle::pid`, whereas ideally we want all of `REC::Particle`'s columns
  // to be filtered
  auto frame_filtered = frame_init
                            .Define( // define a filter column, type std::deque<bool> (to avoid std::vector<bool>)
                                "REC_Particle_EventBuilderFilter",
                                [&](std::vector<int> const& pids)
                                { return algo_eventbuilder_filter.Filter(pids); },
                                {"REC_Particle_pid"})
                            .Define( // apply the filtering column to `REC_Particle_pid`
                                "REC_Particle_pid_good",
                                [](std::vector<int> const& pids, std::deque<bool>& filter)
                                {
                                  std::vector<int> result;
                                  for(std::deque<bool>::size_type i = 0; i < filter.size(); i++) {
                                    if(filter.at(i))
                                      result.push_back(pids.at(i));
                                  }
                                  return result;
                                },
                                {"REC_Particle_pid", "REC_Particle_EventBuilderFilter"});

  // draw
  auto hist = frame_filtered.Histo1D({"pid_filter", "PDG", 6000, -3000, 3000}, "REC_Particle_pid_good");

  // write or hold open
  auto canv = new TCanvas("canv", "canv", 1600, 1200);
  canv->SetGrid(1, 1);
  hist->Draw();
  if(interactive_mode) {
    fmt::print("\n\nShowing plots interactively;\npress ^C to exit.\n\n");
    app->Run();
  }
  else {
    canv->SaveAs("out-iguana-dataframe-example.png");
  }

  return 0;
}
