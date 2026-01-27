#pragma once

#include "iguana/algorithms/Algorithm.h"

namespace iguana::clas12 {

  /// @algo_brief{Simple particle matching by proximity, for example, MC truth-matching}
  /// @algo_type_creator
  /// @begin_doc_config{clas12/MatchParticleProximity}
  /// @config_param{bank_a | string | the particle bank to match from}
  /// @config_param{bank_b | string | the particle bank to match to}
  /// @end_doc
  ///
  /// This algorithm matches one particle bank to another, by smallest proximity, where proximity
  /// is the Euclidean distance in (theta,phi) space.
  ///
  /// By default, it matches `REC::Particle` particles to `MC::Particle` particles. This configuration
  /// is useful for MC files which _lack_ truth-matching banks (`MC::GenMatch` and `MC::RecMatch`);
  /// you should prefer the truth-matching banks instead, if they are available.
  ///
  /// You may also use this algorithm to match `MC::Lund` to `MC::Particle`; in this case, expect match proximity
  /// values to be very close to zero.
  class MatchParticleProximity : public Algorithm
  {

    DEFINE_IGUANA_ALGORITHM(MatchParticleProximity, clas12::MatchParticleProximity)

    private: // hooks
      void ConfigHook() override;
      void StartHook(hipo::banklist& banks) override;
      bool RunHook(hipo::banklist& banks) const override;

    public:

      /// @run_function
      /// @param [in] bank_a the particle bank to match from, _e.g._, `REC::Particle`
      /// @param [in] bank_b the particle bank to match to, _e.g._, `MC::Particle`
      /// @param [out] result_bank `%clas12::MatchParticleProximity`, which will be created
      /// @returns `true` if the created bank is not empty
      bool Run(
          hipo::bank const& bank_a,
          hipo::bank const& bank_b,
          hipo::bank& result_bank) const;

    private:

      // config options
      std::string o_bank_a;
      std::string o_bank_b;

      // banklist indices
      hipo::banklist::size_type b_bank_a;
      hipo::banklist::size_type b_bank_b;
      hipo::banklist::size_type b_result;

      // `b_result` bank item indices
      int i_pindex_a;
      int i_pindex_b;
      int i_proximity;
  };

}
