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

    /// Create the Iguana instance. You may only create one, and you must destroy
    /// it with `iguana_destroy` when you are done. This instance is the _owner_
    /// of algorithm objects.
    void iguana_create_();

    /// Destroy the Iguana instance, along with its algorithms. This must be called
    /// when you are done using Iguana, to free the allocated memory.
    void iguana_destoy_();

    /// Get a pointer to an algorithm. This function is not useful as a Fortran subroutine.
    /// @param [in] algo_idx the algorithm index
    /// @param [out] a pointer to the algorithm, if it exists; if not, `nullptr`
    void iguana_get_algo_(algo_idx_t* algo_idx, Algorithm* algo);

    /// Create an algorithm. Be sure to run `iguana_create_()` before creating any algorithm.
    /// @param [in] algo_name the name of the algorithm
    /// @param [in] algo_name_len @fortran_string_len{algo_name}
    /// @param [out] the algorithm index
    void iguana_algo_create_(char const* algo_name, int algo_name_len, algo_idx_t* algo_idx);

    /// Destroy an algorithm. You probably don't need to call this function, since you can
    /// just destroy all algorithms with `iguana_destroy_()`.
    /// @param [in] algo_idx the algorithm index
    void iguana_algo_destroy_(algo_idx_t* algo_idx);

    /// Start an algorithm by calling `Algorithm::Start`.
    /// @param [in] algo_idx the algorithm index
    void iguana_algo_start_(algo_idx_t* algo_idx);

    /// Stop an algorithm by calling `Algorithm::Stop`.
    /// @param [in] algo_idx the algorithm index
    void iguana_algo_stop_(algo_idx_t* algo_idx);

    /// Set the name of an algorithm.
    /// @param [in] algo_idx the algorithm index
    /// @param [in] name the name
    /// @param [in] name_len @fortran_string_len{name}
    void iguana_algo_set_name_(algo_idx_t* algo_idx, char const* name, int name_len);

    /// Set the log level of an algorithm.
    /// @param [in] algo_idx the algorithm index
    /// @param [in] level the log level
    /// @param [in] level_len @fortran_string_len{level}
    void iguana_algo_set_log_level_(algo_idx_t* algo_idx, char const* level, int level_len);

  }
}
