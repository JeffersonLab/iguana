#include "AlgorithmBindings.h"

#include <stdarg.h>

namespace iguana::bindings {
  extern "C" {

    algo_owner_t __boss; // the boss owns the algorithm instances

    void iguana_print_debug_(char const* format, ...)
    {
      if(__boss.verbose) {
        va_list args;
        va_start(args, format);
        printf("[IGUANA C-BINDINGS] [DEBUG] ");
        vprintf(format, args);
        printf("\n");
        va_end(args);
      }
    }

    void iguana_print_error_(char const* format, ...)
    {
      va_list args;
      va_start(args, format);
      fprintf(stderr, "[IGUANA C-BINDINGS] [ERROR] ");
      vfprintf(stderr, format, args);
      fprintf(stderr, "\n");
      va_end(args);
    }

    Algorithm* iguana_get_algo_(algo_idx_t* algo_idx, bool verbose)
    {
      if(*algo_idx >= 0 && *algo_idx < algo_idx_t(__boss.algos.size())) {
        auto& algo = __boss.algos[*algo_idx];
        if(verbose)
          iguana_print_debug_("        algo %d is at %p", *algo_idx, algo);
        if(algo == nullptr)
          iguana_print_error_("algorithm number %d is NULL", *algo_idx);
        return algo;
      }
      iguana_print_error_("algorithm number %d is not defined", *algo_idx);
      return nullptr;
    }

    void iguana_create_()
    {
      __boss.verbose = false;
    }

    void iguana_start_()
    {
      iguana_print_debug_("starting all algorithm instances...");
      for(const auto& algo: __boss.algos)
        algo->Start();
    }

    void iguana_stop_()
    {
      iguana_stop_and_keep_();
      iguana_destroy_();
    }

    void iguana_stop_and_keep_()
    {
      iguana_print_debug_("stopping all algorithm instances...");
      for(const auto& algo: __boss.algos)
        algo->Stop();
    }

    void iguana_destroy_()
    {
      iguana_print_debug_("destroying all algorithm instances...");
      for(auto& algo : __boss.algos) {
        iguana_print_debug_("  - destroy %p", algo);
        delete algo;
      }
      __boss.algos.clear();
    }

    void iguana_bindings_set_verbose_()
    {
      __boss.verbose = true;
      iguana_print_debug_("enable verbose mode");
    }

    void iguana_bindings_set_quiet_()
    {
      iguana_print_debug_("disable verbose mode");
      __boss.verbose = false;
    }

    void iguana_algo_create_(algo_idx_t* algo_idx, char const* algo_name)
    {
      iguana_print_debug_("creating algorithm '%s' ...", algo_name);
      *algo_idx = __boss.algos.size();
      __boss.algos.push_back(AlgorithmFactory::Create(algo_name).release());
      iguana_print_debug_("... created '%s' algo %d at %p", algo_name, *algo_idx, __boss.algos.back());
    }

    void iguana_algo_start_(algo_idx_t* algo_idx)
    {
      iguana_print_debug_("start algo %d", *algo_idx);
      iguana_get_algo_(algo_idx, true)->Start();
    }

    void iguana_algo_stop_(algo_idx_t* algo_idx)
    {
      iguana_print_debug_("stop algo %d", *algo_idx);
      iguana_get_algo_(algo_idx, true)->Stop();
    }

    void iguana_algo_set_name_(algo_idx_t* algo_idx, char const* name)
    {
      iguana_print_debug_("set algo %d name", *algo_idx);
      iguana_get_algo_(algo_idx, true)->SetName(name);
    }

    void iguana_algo_set_log_level_(algo_idx_t* algo_idx, char const* level)
    {
      iguana_print_debug_("set algo %d log level", *algo_idx);
      iguana_get_algo_(algo_idx, true)->SetLogLevel(level);
    }

  }
}
