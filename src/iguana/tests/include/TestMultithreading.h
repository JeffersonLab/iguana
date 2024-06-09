// test configuration

#include <cassert>
#include <iguana/algorithms/clas12/ZVertexFilter/Algorithm.h>

inline int TestMultithreading(int test_num, int num_events, bool verbose)
{
  if(test_num == 0) {
    fmt::print(stderr, "ERROR: need a test number\n");
    return 1;
  }

  auto algo = std::make_shared<iguana::clas12::ZVertexFilter>();
  algo->SetOption("log", verbose ? "debug" : "info");
  algo->Start();

  switch(test_num) {

  case 1:
    {

      int num_to_wait = 0;
      int run_num = 0;
      std::vector<decltype(run_num)> run_nums = {5000, 5500};
      decltype(run_nums)::size_type which_run_idx = 0;
      for(decltype(num_events) i = 0; i < num_events; i++) {
        if(--num_to_wait <= 0) {
          num_to_wait = 10; // FIXME: generate random number
          which_run_idx = (which_run_idx + 1) % 2;
          run_num = run_nums.at(which_run_idx);
        }
        algo->Reload(run_num);
        assert(run_num == algo->GetRunNum());

      }

      break;
    }

  default:
    fmt::print(stderr, "ERROR: unknown test number '{}'", test_num);
    return 1;
  }
  return 0;
}
