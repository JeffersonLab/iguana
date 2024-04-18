#include "AlgorithmBindings.h"

namespace iguana::bindings {
  extern "C" {

    void iguana_create_()
    {
      __boss.size = 0;
    }

    void iguana_destoy_()
    {
      for(algo_idx_t i=0; i<__boss.size; i++)
        iguana_algo_destroy_(&i);
    }

    void iguana_get_algo_(algo_idx_t* algo_idx, Algorithm* algo)
    {
      if(*algo_idx >= 0 && *algo_idx < __boss.size)
        algo = __boss.algos[*algo_idx];
      else {
        fprintf(stderr, "ERROR [IGUANA]: algorithm number %d is not defined\n", *algo_idx);
        algo = nullptr;
      }
    }

    void iguana_algo_create_(char const* algo_name, int algo_name_len, algo_idx_t* algo_idx)
    {
      if(__boss.size >= MAX_ALGORITHMS || __boss.size < 0) {
        fprintf(stderr, "ERROR: [iguana_algo_create_]: cannot create more than %d algorithms\n", MAX_ALGORITHMS);
        fprintf(stderr, "... or did you forget to call `iguana_create_()`?\n");
        *algo_idx = MAX_ALGORITHMS;
      }
      else {
        __boss.algos[__boss.size++] = AlgorithmFactory::Create(algo_name).release(); // FIXME: string termination
        *algo_idx = __boss.size - 1;
      }
    }

    void iguana_algo_destroy_(algo_idx_t* algo_idx)
    {
      Algorithm* algo = nullptr; // FIXME: almost every function needs these 2 lines.. can we define a preprocessor macro?
      iguana_get_algo_(algo_idx, algo);
      if(algo)
        delete algo;
    }

    void iguana_algo_start_(algo_idx_t* algo_idx)
    {
      Algorithm* algo = nullptr;
      iguana_get_algo_(algo_idx, algo);
      if(algo)
        algo->Start();
    }

    void iguana_algo_stop_(algo_idx_t* algo_idx)
    {
      Algorithm* algo = nullptr;
      iguana_get_algo_(algo_idx, algo);
      if(algo)
        algo->Stop();
    }

    void iguana_algo_set_name_(algo_idx_t* algo_idx, char const* name, int name_len)
    {
      Algorithm* algo = nullptr;
      iguana_get_algo_(algo_idx, algo);
      if(algo)
        algo->SetName(name); // FIXME: string termination
    }

    void iguana_algo_set_log_level_(algo_idx_t* algo_idx, char const* level, int level_len)
    {
      Algorithm* algo = nullptr;
      iguana_get_algo_(algo_idx, algo);
      if(algo)
        algo->SetLogLevel(level); // FIXME: string termination
    }

  }
}
