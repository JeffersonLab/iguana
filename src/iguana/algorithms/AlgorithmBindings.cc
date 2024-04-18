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
        iguana_algo_destroy_(i);
    }

    Algorithm* iguana_get_algo_(algo_idx_t idx)
    {
      if(idx >= 0 && idx < __boss.size)
        return __boss.algos[idx];
      else
        fprintf(stderr, "ERROR [iguana_get_algo_]: algorithm number %d is not defined\n", idx);
      return nullptr;
    }

    algo_idx_t iguana_algo_create_(char const* algo_name)
    {
      if(__boss.size >= MAX_ALGORITHMS || __boss.size < 0) {
        fprintf(stderr, "ERROR: [iguana_algo_create_]: cannot create more than %d algorithms\n", MAX_ALGORITHMS);
        fprintf(stderr, "... or did you forget to call `iguana_create_()`?\n");
        return MAX_ALGORITHMS;
      }
      __boss.algos[__boss.size++] = AlgorithmFactory::Create(algo_name).release();
      return __boss.size - 1;
    }

    void iguana_algo_destroy_(algo_idx_t idx)
    {
      Algorithm* algo = iguana_get_algo_(idx);
      if(algo)
        delete algo;
    }

    void iguana_algo_start_(algo_idx_t idx)
    {
      Algorithm* algo = iguana_get_algo_(idx);
      if(algo)
        algo->Start();
    }

    void iguana_algo_stop_(algo_idx_t idx)
    {
      Algorithm* algo = iguana_get_algo_(idx);
      if(algo)
        algo->Stop();
    }

    void iguana_algo_set_log_level_(algo_idx_t idx, char const* level)
    {
      Algorithm* algo = iguana_get_algo_(idx);
      if(algo)
        algo->SetLogLevel(level);
    }

  }
}
