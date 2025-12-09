#pragma once

#include "iguana/algorithms/Algorithm.h"

namespace iguana::clas12 {

  /// @algo_brief{Simple MC truth matching by proximity}
  /// @algo_type_creator
  /// @begin_doc_config{clas12/MCProximityMatch}
  /// @config_param{particle_bank | string | the particle bank to match from}
  /// @config_param{search_bank | string | the particle bank to match to}
  /// @end_doc
  ///
  /// This algorithm matches one particle bank to another, by proximity; by default, it matches `REC::Particle` particles
  /// to `MC::Particle` particles. This algorithm is useful for MC files which _lack_ truth-matching banks
  /// (`MC::GenMatch` and `MC::RecMatch`); you should prefer the truth-matching banks instead, if they are available.
  class MCProximityMatch : public Algorithm
  {

      DEFINE_IGUANA_ALGORITHM(MCProximityMatch, clas12::MCProximityMatch)

    public:

      void Start(hipo::banklist& banks) override;
      bool Run(hipo::banklist& banks) const override;
      void Stop() override;

      /// @run_function
      /// @param [in] particle_bank the particle bank to match from, typically `REC::Particle`
      /// @param [in] search_bank the particle bank to match to, typically `MC::Particle`
      /// @param [out] result_bank `MC::RecMatch::Proximity`, which will be created
      /// @returns `true` if the created bank is not empty
      bool Run(
          hipo::bank const& particle_bank,
          hipo::bank const& search_bank,
          hipo::bank& result_bank) const;

    private:

      // config options
      std::string o_particle_bank;
      std::string o_search_bank;

      // banklist indices
      hipo::banklist::size_type b_particle_bank;
      hipo::banklist::size_type b_search_bank;
      hipo::banklist::size_type b_result;

      // `b_result` bank item indices
      int i_pindex;
      int i_mcindex;
      int i_proximity;
  };

}
