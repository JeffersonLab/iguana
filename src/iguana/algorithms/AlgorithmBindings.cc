#include "AlgorithmBindings.h"

#include <stdarg.h>

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
      iguana_bindings_log_("destroying all algorithm instances...");
      for(algo_idx_t i=0; i<__boss.size; i++)
        iguana_algo_destroy_(&i);
      iguana_bindings_log_("... algorithm instances destroyed");
    }

    void iguana_bindings_set_verbose_()
    {
      __boss.verbose = true;
      iguana_bindings_log_("enable verbose mode");
    }

    void iguana_bindings_set_quiet_()
    {
      iguana_bindings_log_("disable verbose mode");
      __boss.verbose = false;
    }

    void iguana_bindings_log_(char const* format, ...)
    {
      if(__boss.verbose) {
        va_list args;
        va_start(args, format);
        printf("[IGUANA C-BINDINGS] [LOG] ");
        vprintf(format, args);
        printf("\n");
        va_end(args);
      }
    }

    void iguana_bindings_error_(char const* format, ...)
    {
      if(__boss.verbose) {
        va_list args;
        va_start(args, format);
        fprintf(stderr, "[IGUANA C-BINDINGS] [ERROR] ");
        vfprintf(stderr, format, args);
        fprintf(stderr, "\n");
        va_end(args);
      }
    }

    void iguana_get_algo_(algo_idx_t* algo_idx, Algorithm*& algo)
    {
      if(*algo_idx >= 0 && *algo_idx < __boss.size)
        algo = __boss.algos[*algo_idx];
      else {
        iguana_bindings_error_("algorithm number %d is not defined", *algo_idx);
        algo = nullptr;
      }
    }

    void iguana_algo_create_(algo_idx_t* algo_idx, char const* algo_name)
    {
      iguana_bindings_log_("creating algorithm '%s' ...", algo_name);
      if(__boss.size >= MAX_ALGORITHMS || __boss.size < 0) {
        iguana_bindings_error_("cannot create more than %d algorithms", MAX_ALGORITHMS);
        iguana_bindings_error_("... or did you forget to call `iguana_create_()`?");
        *algo_idx = MAX_ALGORITHMS;
      }
      else {
        __boss.algos[__boss.size] = AlgorithmFactory::Create(algo_name).release();
        *algo_idx = __boss.size;
        iguana_bindings_log_("... created '%s' (%d, %p)", algo_name, *algo_idx, __boss.algos[__boss.size]);
        __boss.size++;
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
        algo->SetName(name);
    }

    void iguana_algo_set_log_level_(algo_idx_t* algo_idx, char const* level)
    {
      Algorithm* algo = nullptr;
      iguana_get_algo_(algo_idx, algo);
      iguana_bindings_log_("set log level of algorithm (%d, %p) to '%s'", *algo_idx, algo, level);
      if(algo)
        algo->SetLogLevel(level);
    }

  }
}
