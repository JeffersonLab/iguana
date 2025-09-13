#pragma once

#include "iguana/algorithms/Algorithm.h"

namespace iguana::clas12 {

  /// @brief_algo Link particle bank to bank `REC::Traj`
  ///
  /// @begin_doc_algo{clas12::TrajLinker | Creator}
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
      bool Run(hipo::banklist& banks) const override;
      void Stop() override;

      /// @run_function
      /// @param [in] bank_particle `REC::Particle`
      /// @param [in] bank_traj `REC::Traj`
      /// @param [out] bank_result `REC::Particle::Traj`, which will be created
      /// @run_function_returns_true
      bool Run(
          hipo::bank const& bank_particle,
          hipo::bank const& bank_traj,
          hipo::bank& bank_result) const;

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
      int i_r1_found;
      int i_r1_x;
      int i_r1_y;
      int i_r1_z;
      int i_r2_found;
      int i_r2_x;
      int i_r2_y;
      int i_r2_z;
      int i_r3_found;
      int i_r3_x;
      int i_r3_y;
      int i_r3_z;
  };

}
