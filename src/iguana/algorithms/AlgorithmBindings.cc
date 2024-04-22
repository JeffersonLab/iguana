#include "AlgorithmBindings.h"

namespace iguana::bindings {
  extern "C" {

    /// The Iguana singleton instance, owning up to `MAX_ALGORITHMS` algorithms.
    /// Do not access or modify its contents directly, unless you have a good reason to;
    /// instead, use the functions in this file.
    algo_boss_t __boss;

    void iguana_create_()
    {
      __boss.size = 0;
      __boss.verbose = false;
    }

    void iguana_destroy_()
    {
      for(algo_idx_t i=0; i<__boss.size; i++)
        iguana_algo_destroy_(&i);
    }

    void iguana_set_verbose_bindings_()
    {
      printf("[IGUANA C-BINDING] enable verbose mode\n");
      __boss.verbose = true;
    }

    void iguana_set_quiet_bindings_()
    {
      printf("[IGUANA C-BINDING] disable verbose mode\n");
      __boss.verbose = false;
    }

    void iguana_get_algo_(algo_idx_t* algo_idx, Algorithm* algo)
    {
      if(*algo_idx >= 0 && *algo_idx < __boss.size)
        algo = __boss.algos[*algo_idx];
      else {
        fprintf(stderr, "[IGUANA C-BINDING] ERROR: algorithm number %d is not defined\n", *algo_idx);
        algo = nullptr;
      }
    }

    void iguana_algo_create_(algo_idx_t* algo_idx, char const* algo_name)
    {
      printf("[IGUANA C-BINDING] create algorithm: '%s'\n", algo_name);
      if(__boss.size >= MAX_ALGORITHMS || __boss.size < 0) {
        fprintf(stderr, "[IGUANA C-BINDING] ERROR: cannot create more than %d algorithms\n", MAX_ALGORITHMS);
        fprintf(stderr, "[IGUANA C-BINDING]        ... or did you forget to call `iguana_create_()`?\n");
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

    void iguana_algo_set_name_(algo_idx_t* algo_idx, char const* name)
    {
      Algorithm* algo = nullptr;
      iguana_get_algo_(algo_idx, algo);
      if(algo)
        algo->SetName(name); // FIXME: string termination
    }

    void iguana_algo_set_log_level_(algo_idx_t* algo_idx, char const* level)
    {
      printf("[IGUANA C-BINDING] set log level of algorithm %d to '%s'\n", *algo_idx, level);
      Algorithm* algo = nullptr;
      iguana_get_algo_(algo_idx, algo);
      if(algo)
        algo->SetLogLevel(level); // FIXME: string termination
    }

  }
}
