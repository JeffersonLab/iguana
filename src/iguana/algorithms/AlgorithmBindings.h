#pragma once

#include "Algorithm.h"

namespace iguana::bindings {
  extern "C" {

  /// `Algorithm` instance index type
  typedef int algo_idx_t;

  /// `Algorithm` instance owner type
  typedef struct
  {
      /// A list of `Algorithm` instance pointers
      std::vector<Algorithm*> algos;
      /// Control printout verbosity
      bool verbose;
  } algo_owner_t;

  /// Print a log message, only if `iguana_bindings_set_verbose_` was called.
  /// @warning This function is not for Fortran
  /// @param format `printf` arguments
  void iguana_print_log_(char const* format, ...);

  /// Print an error message.
  /// @warning This function is not for Fortran
  /// @param format `printf` arguments
  void iguana_print_error_(char const* format, ...);

  /// Get a pointer to an algorithm.
  /// @warning This function is not for Fortran
  /// @param [in] algo_idx the algorithm index
  /// @param [in] verbose enable verbose printout
  /// @returns a pointer to the algorithm, if it exists; if not, `nullptr`
  Algorithm* iguana_get_algo_(algo_idx_t* algo_idx, bool verbose = false);

  /// Create the Iguana instance. You may only create one, and you must destroy
  /// it with `iguana_stop` or `iguana_destroy` when you are done. This instance is the _owner_
  /// of algorithm objects.
  void iguana_create_();

  /// Set a custom configuration file for _all_ algorithms
  /// @param [in] name the configuration file name
  void iguana_set_config_file_(char const* name);

  /// Set a custom configuration file directory for _all_ algorithms
  /// @param [in] name the directory name
  void iguana_set_config_dir_(char const* name);

  /// Start all created algorithm instances,
  /// calling `Algorithm::Start` on each.
  void iguana_start_();

  /// Stop all created algorithm instances, calling `Algorithm::Stop` on each,
  /// and free the allocated memory.
  /// @see iguana_stop_and_keep_()
  void iguana_stop_();

  /// Stop all created algorithm instances,
  /// but do not destroy them
  void iguana_stop_and_keep_();

  /// Destroy the Iguana instance, along with its algorithms. This must be called
  /// when you are done using Iguana, to free the allocated memory.
  void iguana_destroy_();

  /// Enable additional runtime printouts for these binding functions. This setting
  /// is _not_ related to algorithm log levels.
  /// @see `iguana_bindings_set_quiet_`
  void iguana_bindings_set_verbose_();

  /// Disable additional runtime printouts for these binding functions. This setting
  /// is _not_ related to algorithm log levels.
  /// @see `iguana_bindings_set_verbose_`
  void iguana_bindings_set_quiet_();

  /// Create an algorithm. Be sure to run `iguana_create_()` before creating any algorithm.
  /// @param [out] algo_idx the algorithm index
  /// @param [in] algo_name the name of the algorithm
  void iguana_algo_create_(algo_idx_t* algo_idx, char const* algo_name);

  /// Set the name of an algorithm.
  /// @param [in] algo_idx the algorithm index
  /// @param [in] name the name
  void iguana_algo_set_name_(algo_idx_t* algo_idx, char const* name);

  /// Set the log level of an algorithm.
  /// @param [in] algo_idx the algorithm index
  /// @param [in] level the log level
  void iguana_algo_set_log_level_(algo_idx_t* algo_idx, char const* level);

  /// Set a custom configuration file for this algorithm
  /// @param [in] algo_idx the algorithm index
  /// @param [in] name the configuration file name
  void iguana_algo_set_config_file_(algo_idx_t* algo_idx, char const* name);

  /// Set a custom configuration file directory for this algorithm
  /// @param [in] algo_idx the algorithm index
  /// @param [in] name the directory name
  void iguana_algo_set_config_dir_(algo_idx_t* algo_idx, char const* name);

  /// Start an algorithm by calling `Algorithm::Start`.
  /// @param [in] algo_idx the algorithm index
  void iguana_algo_start_(algo_idx_t* algo_idx);

  /// Stop an algorithm by calling `Algorithm::Stop`.
  /// @param [in] algo_idx the algorithm index
  void iguana_algo_stop_(algo_idx_t* algo_idx);

  /// Get the configuration file installation prefix
  /// @param [in,out] out will be set to the prefix
  void iguana_getconfiginstallationprefix_(char* out);
  }
}
