#include "Algorithm.h"

namespace iguana {
  extern "C" {

    void* iguana_algo_create(char const* algo_name)
    {
      return (void*) AlgorithmFactory::Create(algo_name).release();
    }

    void iguana_algo_start(void* algo)
    {
      reinterpret_cast<Algorithm*>(algo)->Start();
    }

    void iguana_algo_stop(Algorithm* algo)
    {
      algo->Stop();
    }

    void iguana_algo_destroy(Algorithm* algo)
    {
      algo->~Algorithm();
      // FIXME: should `delete algo` also be called?
    }

    void iguana_algo_stop_and_destroy(Algorithm* algo)
    {
      iguana_algo_stop(algo);
      iguana_algo_destroy(algo);
    }

  }
}
