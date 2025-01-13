#pragma once

#include "iguana/algorithms/Algorithm.h"
#include "iguana/services/ConcurrentParam.h"

namespace iguana::clas12 {

  /// @brief_algo Filter using Quality Assurance Database (QADB)
  ///
  /// @begin_doc_algo{clas12::QADB | EventFilter}
  /// @input_banks{RUN::config}
  /// @output_banks{REC::Particle}
  /// @end_doc
  ///
  /// @begin_doc_config
  /// @config_param{datasets | list[string] | the list of QADB datasets to include (see below)}
  /// @config_param{qadb_dir | string | custom QADB directory; if not set, defaults to environment variable `$QADB`}
  /// @config_param{create_bank | bool | if `true`, create the output bank with QADB information for this event}
  /// @end_doc
  ///
  /// This algorithm is an "EventFilter" type, which uses the return value of `iguana::Algorithm::Run` to
  /// indicate whether the whole event is filtered or not.
  ///
  /// This algorithm requires the QADB to be installed. The environment variable `$QADB` is assumed to point
  /// to the QADB installation. Alternatively, use the configuration variable `qadb_dir` if you do not
  /// want to use `$QADB`.
  ///
  /// The QADB is defined for various datasets, and you must choose which dataset(s) to load by setting the
  /// `datasets` configuration parameter; it is a list of `string`s, where each can be either:
  /// - a dataset name, where the list of datasets is found in the QADB documentation
  ///   - in this case, the QADB files should be within `$QADB` (or `qadb_dir`) within `qadb/<dataset>/`
  /// - the full path to a QADB `json` file
  ///
  /// This algorithm has the option `create_bank` to control whether or not an output bank is created,
  /// which contains information from the QADB about the QA bin that contains the current event.
  class QADB : public Algorithm
  {

      DEFINE_IGUANA_ALGORITHM(QADB, clas12::QADB)

    public:

      void Start(hipo::banklist& banks) override;
      void Run(hipo::banklist& banks) const override;
      void Stop() override;

      /// @action_function{reload} prepare the event
      /// @when_to_call{for each event}
      /// @param runnum the run number
      /// @param evnum the event number
      /// @returns the key to be used in `::Filter`
      concurrent_key_t PrepareEvent(int const runnum, int const evnum) const;

      /// @action_function{scalar filter} checks if the Z Vertex is within specified bounds if pid is one for which the filter should be applied to.;
      /// Cuts applied to particles in FD or CD (ie not in FT).
      /// @when_to_call{for each particle}
      /// @param key the return value of `::PrepareEvent`
      /// @returns `true` if `zvertex` is within specified bounds
      bool Filter(concurrent_key_t const key) const;

      /// @param key the return value of `::PrepareEvent`
      /// @returns the current run number
      int GetRunNum(concurrent_key_t const key) const;

      /// @param key the return value of `::PrepareEvent`
      /// @returns the current QA bin number
      int GetBinNum(concurrent_key_t const key) const;

    private:
      hipo::banklist::size_type b_particle, b_config;

      // Reload function
      void Reload(int const runnum, int const evnum, concurrent_key_t key) const;

      // configuration options
      std::vector<std::string> o_datasets;
      std::string o_qadb_dir;
      bool o_create_bank{false};

      // concurrent params
      mutable std::unique_ptr<ConcurrentParam<int>> o_runnum;
      mutable std::unique_ptr<ConcurrentParam<int>> o_binnum;

  };

}
