#pragma once

#include "iguana/algorithms/Algorithm.h"

namespace iguana::clas12 {

  /// @brief_algo Link particle bank to bank `REC::Traj`
  ///
  /// @begin_doc_algo{clas12::TrajLinker | Creator}
  /// @input_banks{REC::Particle, REC::Traj}
  /// @output_banks{%REC::Particle::Traj}
  /// @end_doc
  ///
  /// This algorithm reads `REC::Traj` and produces a new bank, `REC::Particle::Traj`,
  /// to make it easier to access commonly used `REC::Traj` information for each particle.
  ///
  /// If this algorithm does not provide information you need, ask the maintainers or open a pull request.
  ///
  /// @creator_note
  class TrajLinker : public Algorithm
  {

      DEFINE_IGUANA_ALGORITHM(TrajLinker, clas12::TrajLinker)

    public:

      void Start(hipo::banklist& banks) override;
      void Run(hipo::banklist& banks) const override;
      void Stop() override;

      /// @returns the DC sector given (x,y,z), or `-1` if failed
      /// @param x x-position
      /// @param y y-position
      /// @param z z-position
      int GetSector(float const& x, float const& y, float const& z) const;

    private:

      /// `hipo::banklist` indices
      hipo::banklist::size_type b_particle;
      hipo::banklist::size_type b_traj;
      hipo::banklist::size_type b_result;

      // `b_result` bank item indices
      int i_pindex;
      int i_sector;
      int i_r1x;
      int i_r1y;
      int i_r1z;
      int i_r2x;
      int i_r2y;
      int i_r2z;
      int i_r3x;
      int i_r3y;
      int i_r3z;
  };

}
