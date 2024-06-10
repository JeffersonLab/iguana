// test configuration

#include <iguana/algorithms/clas12/ZVertexFilter/Algorithm.h>
#include <thread>
#include <cassert>
#include <mutex>

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

      std::vector<int> run_nums = {5000, 5500};

      std::atomic<int> oa_local;
      oa_local.store(0);

      std::mutex mlock;
      int locked_run_num = 0;

      auto event_loop = [&mlock, &locked_run_num, &run_nums, &oa_local, &num_events, &verbose](int thread_num) {

        int num_to_wait = 0;
        int thread_run_num = 0;
        decltype(run_nums)::size_type which_run_idx = 0;

        for(decltype(num_events) i = 0; i < num_events; i++) {

          // change run number
          if(--num_to_wait <= 0) {
            num_to_wait = 10; // FIXME: generate random number
            which_run_idx = (which_run_idx + 1) % 2;
            thread_run_num = run_nums.at(which_run_idx);
            if(verbose) fmt::print("thread {}: ----- change run to {}\n", thread_num, thread_run_num);
          }
          else thread_run_num = run_nums.at(which_run_idx);

          // emulate Reload
          mlock.lock();
          if(locked_run_num != thread_run_num) {
            locked_run_num = thread_run_num;
          }
          /*mlock.unlock();*/ // don't unlock/re-lock before "load", otherwise we have a data race

          // emulate load
          /*mlock.lock();*/
          auto loaded_run_num = locked_run_num;
          mlock.unlock();

          fmt::print("thread {}: i={} thread_run_num={} locked_run_num={}\n", thread_num, i, thread_run_num, loaded_run_num);
          if(thread_run_num != loaded_run_num) {
            fmt::print("ERROR: thread {}: i={} thread_run_num={} locked_run_num={}\n", thread_num, i, thread_run_num, locked_run_num);
            fmt::print("\n");
            fmt::print("\n");
            throw std::runtime_error("data race!");
          }

        }
      };

      // auto event_loop = [&run_nums, &algo, &oa_local, &num_events, &verbose](int thread_num) {
      //   int num_to_wait = 0;
      //   int thread_run_num = 0;
      //   decltype(run_nums)::size_type which_run_idx = 0;
      //   for(decltype(num_events) i = 0; i < num_events; i++) {
      //     if(--num_to_wait <= 0) {
      //       num_to_wait = 10; // FIXME: generate random number
      //       which_run_idx = (which_run_idx + 1) % 2;
      //       thread_run_num = run_nums.at(which_run_idx);
      //       if(verbose)
      //         fmt::print("thread {}: ----- change run to {}\n", thread_num, thread_run_num);
      //       // algo->Reload(thread_run_num);
      //       oa_local.store(thread_run_num);
      //     }
      //     thread_run_num = run_nums.at(which_run_idx);
      //     // int local_atomic_run_num = thread_run_num;
      //     // oa_local.compare_exchange_strong(local_atomic_run_num, thread_run_num);
      //     // auto local_atomic_run_num = oa_local.exchange(thread_run_num);
      //     auto local_atomic_run_num = oa_local.load();
      //     auto algo_run_num = algo->GetRunNum();
      //     // auto local_atomic_run_num = oa_local.load();
      //     if(verbose)
      //       fmt::print("thread {}: i={} thread_run_num={} algo_run_num={} local_atomic_run_num={}\n", thread_num, i, thread_run_num, algo_run_num, local_atomic_run_num);
      //     // assert(thread_run_num == algo_run_num);
      //     if(local_atomic_run_num != 0) {
      //       assert(thread_run_num == local_atomic_run_num);
      //     }
      //   }
      // };

      std::thread t1(event_loop, 1);
      std::thread t2(event_loop, 2);
      t1.join();
      t2.join();

      break;
    }

  default:
    fmt::print(stderr, "ERROR: unknown test number '{}'", test_num);
    return 1;
  }
  return 0;
}
