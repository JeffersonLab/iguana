#include "Algorithm.h"

namespace iguana::bindings {
  extern "C" {

    /// Create an algorithm
    /// @param algo_name the name of the algorithm
    /// @returns a pointer to the algorithm
    void* iguana_algo_create(char const* algo_name)
    {
      return (void*) AlgorithmFactory::Create(algo_name).release();
    }

    /// Start an algorithm by calling `Algorithm::Start`
    /// @param algo the algorithm
    void iguana_algo_start(Algorithm* algo)
    {
      algo->Start();
    }

    /// Stop an algorithm by calling `Algorithm::Stop`
    /// @param algo the algorithm
    void iguana_algo_stop(Algorithm* algo)
    {
      algo->Stop();
    }

    /// Destroy an algorithm by calling its destructor
    /// @param algo the algorithm
    void iguana_algo_destroy(Algorithm* algo)
    {
      algo->~Algorithm();
      // FIXME: should `delete algo` also be called?
    }

    /// Call `iguana::bindings::iguana_algo_stop` then `iguana::bindings::iguana_algo_destroy`
    /// @param algo the algorithm
    void iguana_algo_stop_and_destroy(Algorithm* algo)
    {
      iguana_algo_stop(algo);
      iguana_algo_destroy(algo);
    }

    /// Set the log level of this algorithm
    /// @param algo the algorithm
    /// @param level the log level
    void iguana_algo_set_log_level(Algorithm* algo, char const* level)
    {
      algo->SetLogLevel(level);
    }

  }
}
