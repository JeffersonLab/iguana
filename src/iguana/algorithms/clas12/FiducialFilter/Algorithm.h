#pragma once

#include "iguana/algorithms/Algorithm.h"

namespace iguana::clas12 {

  /// @brief_algo Filter the `REC::Particle` bank by applying DC (drift chamber) and ECAL (electromagnetic calorimeter) fiducial cuts
  ///
  /// Currently these are the "legacy" Pass 1 fiducial cuts tuned for Run Group A.
  ///
  /// @begin_doc_algo{clas12::FiducialFilter | Filter}
  /// Pass 1 Filter:
  /// @input_banks{REC::Particle, REC::Particle::Traj, REC::Particle::Calorimeter, RUN::config}
  /// @output_banks{REC::Particle}
  /// @end_doc
  ///
  /// The banks `REC::Particle::Traj` and `REC::Particle::Calorimeter` are created by
  /// `iguana::clas12::TrajLinker` and `iguana::clas12::CalorimeterLinker`, respectively,
  /// for getting values from `REC::Calorimeter` and `REC::Traj` for each particle.
  ///
  /// @begin_doc_config{clas12/FiducialFilter}
  /// @config_param{pass | int | cook type to use for assigning fiducial cuts}
  /// @config_param{ecal_cut_level | string | cut level for ECAL cuts, one of: loose, medium, tight}
  /// @end_doc
  class FiducialFilter : public Algorithm
  {

      DEFINE_IGUANA_ALGORITHM(FiducialFilter, clas12::FiducialFilter)

    public:

      enum CutLevel {
        loose,
        medium,
        tight
      };

      void Start(hipo::banklist& banks) override;
      void Run(hipo::banklist& banks) const override;
      void Stop() override;

      /// @action_function{scalar filter} top-level fiducial cut for RG-A Pass 1
      /// @param pcal_sector PCAL sector
      /// @param pcal_lu PCAL lu
      /// @param pcal_lv PCAL lv
      /// @param pcal_lw PCAL lw
      /// @param ecin_sector ECIN sector
      /// @param ecin_lu ECIN lu
      /// @param ecin_lv ECIN lv
      /// @param ecin_lw ECIN lw
      /// @param ecout_sector ECOUT sector
      /// @param ecout_lu ECOUT lu
      /// @param ecout_lv ECOUT lv
      /// @param ecout_lw ECOUT lw
      /// @param dc_sector DC sector
      /// @param dc_r1x DC region 1 x
      /// @param dc_r1y DC region 1 y
      /// @param dc_r1z DC region 1 z
      /// @param dc_r2x DC region 2 x
      /// @param dc_r2y DC region 2 y
      /// @param dc_r2z DC region 2 z
      /// @param dc_r3x DC region 3 x
      /// @param dc_r3y DC region 3 y
      /// @param dc_r3z DC region 3 z
      /// @param torus the toris magnetic field sign
      /// @param pid the PDG of the particle
      /// @returns `true` if passes fiducial cuts
      bool FilterRgaPass1(
          int const pcal_sector,
          float const pcal_lu,
          float const pcal_lv,
          float const pcal_lw,
          int const ecin_sector,
          float const ecin_lu,
          float const ecin_lv,
          float const ecin_lw,
          int const ecout_sector,
          float const ecout_lu,
          float const ecout_lv,
          float const ecout_lw,
          int const dc_sector,
          float const dc_r1x,
          float const dc_r1y,
          float const dc_r1z,
          float const dc_r2x,
          float const dc_r2y,
          float const dc_r2z,
          float const dc_r3x,
          float const dc_r3y,
          float const dc_r3z,
          float const torus,
          int const pid) const;

      /// @action_function{scalar filter} filter using DC XY fiducial cut
      /// @param sector DC sector
      /// @param r1x DC region 1 x
      /// @param r1y DC region 1 y
      /// @param r1z DC region 1 z
      /// @param r2x DC region 2 x
      /// @param r2y DC region 2 y
      /// @param r2z DC region 2 z
      /// @param r3x DC region 3 x
      /// @param r3y DC region 3 y
      /// @param r3z DC region 3 z
      /// @param torus the toris magnetic field sign
      /// @param pid the PDG of the particle
      /// @returns `true` if passes fiducial cuts
      bool FilterDcXY(
          int const sector,
          float const r1x,
          float const r1y,
          float const r1z,
          float const r2x,
          float const r2y,
          float const r2z,
          float const r3x,
          float const r3y,
          float const r3z,
          float const torus,
          int const pid) const;

      /// @action_function{scalar filter} filter using DC theta-phi fiducial cut
      /// @param sector DC sector
      /// @param r1x DC region 1 x
      /// @param r1y DC region 1 y
      /// @param r1z DC region 1 z
      /// @param r2x DC region 2 x
      /// @param r2y DC region 2 y
      /// @param r2z DC region 2 z
      /// @param r3x DC region 3 x
      /// @param r3y DC region 3 y
      /// @param r3z DC region 3 z
      /// @param torus the toris magnetic field sign
      /// @param pid the PDG of the particle
      /// @returns `true` if passes fiducial cuts
      bool FilterDcThetaPhi(
          int const sector,
          float const r1x,
          float const r1y,
          float const r1z,
          float const r2x,
          float const r2y,
          float const r2z,
          float const r3x,
          float const r3y,
          float const r3z,
          float const torus,
          int const pid) const;

  //////////////////////////////

    private:

      /// `hipo::banklist` 
      hipo::banklist::size_type b_particle;
      hipo::banklist::size_type b_traj;
      hipo::banklist::size_type b_cal;
      hipo::banklist::size_type b_config;

      /// Pass Reconstruction
      int o_pass = 1;

      /// ECAL cut level
      CutLevel o_ecal_cut_level;

  };

}
