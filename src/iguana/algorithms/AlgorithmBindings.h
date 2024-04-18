#pragma once

#include "Algorithm.h"

#define MAX_ALGORITHMS 30

namespace iguana::bindings{
  extern "C" {

    typedef int algo_idx_t;
    typedef struct
    {
      Algorithm* algos[MAX_ALGORITHMS];
      algo_idx_t size;
    } algo_boss_t;

    /// The primary algorithm instance. Since this is a global variable, this is the only one
    algo_boss_t __boss;

    /// Create the Iguana instance. You may only create one, and you must destroy
    /// it with `iguana_destroy` when you are done. This instance is the _owner_
    /// of algorithm objects.
    void iguana_create_();

    /// Destroy the Iguana instance, along with its algorithms.
    void iguana_destoy_();

    /// Get a pointer to an algorithm
    /// @param idx the algorithm index
    /// @returns a pointer to the algorithm, if it exists; if not, `nullptr`
    Algorithm* iguana_get_algo_(algo_idx_t idx);

    /// Create an algorithm
    /// @param algo_name the name of the algorithm
    /// @returns the algorithm index
    algo_idx_t iguana_algo_create_(char const* algo_name);

    /// Destroy an algorithm
    /// @param idx the algorithm index
    void iguana_algo_destroy_(algo_idx_t idx);

    /// Start an algorithm by calling `Algorithm::Start`
    /// @param idx the algorithm index
    void iguana_algo_start_(algo_idx_t idx);

    /// Stop an algorithm by calling `Algorithm::Stop`
    /// @param idx the algorithm index
    void iguana_algo_stop_(algo_idx_t idx);

    /// Set the log level of an algorithm
    /// @param idx the algorithm index
    /// @param level the log level
    void iguana_algo_set_log_level_(algo_idx_t idx, char const* level);

  }
}
