#pragma once

#include "iguana/algorithms/Algorithm.h"

namespace iguana::clas12::rga {

  /// @algo_brief{Filter the `REC::Particle` bank by applying DC (drift chamber) and ECAL (electromagnetic calorimeter) fiducial cuts}
  /// @algo_type_filter
  ///
  /// Currently these are the "legacy" Pass 1 fiducial cuts tuned for Run Group A.
  ///
  /// The following **additional banks** are needed:
  /// - if configuration option `pass` is `1`:
  ///   - `REC::Particle::Traj`, created by algorithm `iguana::clas12::TrajLinker`
  ///   - `REC::Particle::Calorimeter`, created by algorithm `iguana::clas12::CalorimeterLinker`
  ///
  /// @begin_doc_config{clas12/rga/FiducialFilterPass1}
  /// @config_param{pass | int | cook type to use for assigning fiducial cuts}
  /// @config_param{pcal_electron_cut_level | string | cut level for PCAL homogeneous cuts for electrons and positrons, one of: loose, medium, tight}
  /// @config_param{pcal_photon_cut_level | string | cut level for PCAL homogeneous cuts for photons, one of: loose, medium, tight}
  /// @config_param{enable_pcal_cuts | int | enable (1) or disable (0) PCAL cuts }
  /// @config_param{enable_dc_cuts | int | enable (1) or disable (0) DC cuts }
  /// @end_doc
  class FiducialFilterPass1 : public Algorithm
  {

      DEFINE_IGUANA_ALGORITHM(FiducialFilterPass1, clas12::rga::FiducialFilterPass1)

    private: // hooks
      void ConfigHook() override;
      void StartHook(hipo::banklist& banks) override;
      bool RunHook(hipo::banklist& banks) const override;

    public:

      /// cut levels, currently only used for PCAL cuts
      enum CutLevel {
        loose,
        medium,
        tight
      };

      /// @run_function
      /// @param [in,out] particleBank `REC::Particle`, which will be filtered
      /// @param [in] configBank `RUN::config`
      /// @param [in] trajBank `REC::Particle::Traj`, created by algorithm `clas12::TrajLinker`
      /// @param [in] calBank `REC::Particle::Calorimeter`, created by algorithm `clas12::CalorimeterLinker`
      /// @returns `false` if all particles are filtered out
      bool Run(
          hipo::bank& particleBank,
          hipo::bank const& configBank,
          hipo::bank const& trajBank,
          hipo::bank const& calBank) const;

      /// @action_function{scalar filter} top-level fiducial cut for RG-A Pass 1
      /// @param pcal_sector PCAL sector
      /// @param pcal_lv PCAL lv
      /// @param pcal_lw PCAL lw
      /// @param pcal_found if PCAL info exists for this particle, this should be true
      /// @param dc_sector DC sector
      /// @param dc_r1_x DC region 1 x
      /// @param dc_r1_y DC region 1 y
      /// @param dc_r1_z DC region 1 z
      /// @param dc_r1_found if DC region 1 info exists for this particle, this should be true
      /// @param dc_r2_x DC region 2 x
      /// @param dc_r2_y DC region 2 y
      /// @param dc_r2_z DC region 2 z
      /// @param dc_r2_found if DC region 2 info exists for this particle, this should be true
      /// @param dc_r3_x DC region 3 x
      /// @param dc_r3_y DC region 3 y
      /// @param dc_r3_z DC region 3 z
      /// @param dc_r3_found if DC region 3 info exists for this particle, this should be true
      /// @param torus the torus magnetic field sign
      /// @param pid the PDG of the particle
      /// @returns `true` if passes fiducial cuts
      bool FilterRgaPass1(
          int const pcal_sector,
          float const pcal_lv,
          float const pcal_lw,
          bool const pcal_found,
          int const dc_sector,
          float const dc_r1_x,
          float const dc_r1_y,
          float const dc_r1_z,
          bool const dc_r1_found,
          float const dc_r2_x,
          float const dc_r2_y,
          float const dc_r2_z,
          bool const dc_r2_found,
          float const dc_r3_x,
          float const dc_r3_y,
          float const dc_r3_z,
          bool const dc_r3_found,
          float const torus,
          int const pid) const;

      /// @action_function{scalar filter} EC hit position homogeneous cut on lv and lw
      /// @param pcal_sector PCAL sector
      /// @param lv PCAL lv
      /// @param lw PCAL lw
      /// @param torus the torus magnetic field sign
      /// @param pid the PDG of the particle
      bool FilterPcalHomogeneous(
          int const pcal_sector,
          float const lv,
          float const lw,
          float const torus,
          int const pid) const;

      /// @action_function{scalar filter} filter using DC XY fiducial cut
      /// @param dc_sector DC sector
      /// @param r1_x DC region 1 x
      /// @param r1_y DC region 1 y
      /// @param r1_z DC region 1 z
      /// @param r2_x DC region 2 x
      /// @param r2_y DC region 2 y
      /// @param r2_z DC region 2 z
      /// @param r3_x DC region 3 x
      /// @param r3_y DC region 3 y
      /// @param r3_z DC region 3 z
      /// @param torus the torus magnetic field sign
      /// @param pid the PDG of the particle
      /// @returns `true` if passes fiducial cuts
      bool FilterDcXY(
          int const dc_sector,
          float const r1_x,
          float const r1_y,
          float const r1_z,
          float const r2_x,
          float const r2_y,
          float const r2_z,
          float const r3_x,
          float const r3_y,
          float const r3_z,
          float const torus,
          int const pid) const;

      /// @action_function{scalar filter} filter using DC theta-phi fiducial cut
      /// @param dc_sector DC sector
      /// @param r1_x DC region 1 x
      /// @param r1_y DC region 1 y
      /// @param r1_z DC region 1 z
      /// @param r2_x DC region 2 x
      /// @param r2_y DC region 2 y
      /// @param r2_z DC region 2 z
      /// @param r3_x DC region 3 x
      /// @param r3_y DC region 3 y
      /// @param r3_z DC region 3 z
      /// @param torus the torus magnetic field sign
      /// @param pid the PDG of the particle
      /// @returns `true` if passes fiducial cuts
      bool FilterDcThetaPhi(
          int const dc_sector,
          float const r1_x,
          float const r1_y,
          float const r1_z,
          float const r2_x,
          float const r2_y,
          float const r2_z,
          float const r3_x,
          float const r3_y,
          float const r3_z,
          float const torus,
          int const pid) const;

    private: // methods

      /// @param level the cut level string
      /// @returns `CutLevel` associated to the input
      CutLevel ParseCutLevel(std::string const& level) const;

    private: // members

      // `hipo::banklist`
      hipo::banklist::size_type b_particle;
      hipo::banklist::size_type b_traj;
      hipo::banklist::size_type b_cal;
      hipo::banklist::size_type b_config;

      // options
      CutLevel o_pcal_electron_cut_level;
      CutLevel o_pcal_photon_cut_level;
      bool o_enable_pcal_cuts;
      bool o_enable_dc_cuts;
  };

}
