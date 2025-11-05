#pragma once

#include "iguana/algorithms/Algorithm.h"

namespace iguana::clas12 {

  /// @algo_brief{Simple MC truth matching by proximity}
  /// @algo_type_creator
  ///
  /// This algorithm matches reconstructed and generated particles by proximity, and is only meant for _older_ MC
  /// files which _lack_ truth-matching banks (`MC::GenMatch` and `MC::RecMatch`). You should prefer the truth-matching
  /// banks instead, if they are available.
  class MCProximityMatch : public Algorithm
  {

      DEFINE_IGUANA_ALGORITHM(MCProximityMatch, clas12::MCProximityMatch)

    public:

      void Start(hipo::banklist& banks) override;
      bool Run(hipo::banklist& banks) const override;
      void Stop() override;

      /// @run_function
      /// @param [in] rec_particle_bank `REC::Particle` bank, the reconstructed particles
      /// @param [in] mc_particle_bank `MC::Particle` bank, the generated particles
      /// @param [out] result_bank `MC::RecMatch::Proximity`, which will be created
      /// @run_function_returns_true
      bool Run(
          hipo::bank const& rec_particle_bank,
          hipo::bank const& mc_particle_bank,
          hipo::bank& result_bank) const;

    private:

      // banklist indices
      hipo::banklist::size_type b_rec_particle_bank;
      hipo::banklist::size_type b_mc_particle_bank;
      hipo::banklist::size_type b_result;

      // `b_result` bank item indices
      int i_pindex;
      int i_mcindex;
      int i_proximity;
  };

}
