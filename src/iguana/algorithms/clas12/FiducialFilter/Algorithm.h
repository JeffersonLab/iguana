#pragma once

#include "iguana/algorithms/Algorithm.h"
#include "RgaPass1Ecal.h"

namespace iguana::clas12 {

  /// @brief_algo Filter the `REC::Particle` bank by applying DC (drift chamber) and ECAL (electromagnetic calorimeter) fiducial cuts
  ///
  /// Currently these are the "legacy" Pass 1 fiducial cuts tuned for Run Group A.
  ///
  /// @begin_doc_algo{clas12::FiducialFilter | Filter}
  /// @input_banks{REC::Particle, REC::Traj, REC::Calorimeter, RUN::config}
  /// @output_banks{REC::Particle}
  /// @end_doc
  ///
  /// @begin_doc_config{clas12/FiducialFilter}
  /// @config_param{pass | int | cook type to use for assigning fiducial cuts}
  /// @config_param{ecal_cut_level | string | cut level for ECAL cuts, one of: loose, medium, tight}
  /// @end_doc
  class FiducialFilter : public Algorithm
  {

      DEFINE_IGUANA_ALGORITHM(FiducialFilter, clas12::FiducialFilter)

    public:

      void Start(hipo::banklist& banks) override;
      void Run(hipo::banklist& banks) const override;
      void Stop() override;

      /// structure to hold `REC::Traj` data
      struct traj_row_data {
        /// @doxygen_off
        double x1 = -999;
        double x2 = -999;
        double x3 = -999;
        double y1 = -999;
        double y2 = -999;
        double y3 = -999;
        double z1 = -999;
        double z2 = -999;
        double z3 = -999;
        int sector= -1;
        /// @doxygen_on
      };

      /// **Method**: Gets trajectory data for particles in the event
      /// @param bank the bank to get data from
      /// @returns a map with keys as particle indices (pindex) and values as traj_row_data structs
      static std::map<int, FiducialFilter::traj_row_data> GetTrajMap(hipo::bank const &bank);

      /// **Method**: Gets trajectory data for particles in the event
      /// @param x Drift Chamber x coord
      /// @param y Drift Chamber y coord
      /// @param z Drift Chamber z coord
      /// @returns sector number in DC
      static int determineSectorDC(float x, float y, float z);

    private:

      /// RG-A Pass 1 ECAL cuts
      std::unique_ptr<RgaPass1Ecal> m_legacy_ecal_cuts;

      /// **Method**: checks if the particle passes fiducial cuts
      /// @param traj_row data struct of the particle in REC::Traj
      /// @param torus toroidal magnetic field sign
      /// @param pid pid of the particle
      /// @returns `true` if passes fiducial cuts
      bool Filter(FiducialFilter::traj_row_data const traj_row, float const torus, int const pid) const;

      
      /// **Method**: Examines XY fiducial cut for pass1
      /// @param traj_row data struct of the particle in REC::Traj
      /// @param torus toroidal magnetic field sign
      /// @param pid pid of the particle
      /// @returns `true` if passes fiducial cuts
      bool DC_fiducial_cut_XY_pass1(FiducialFilter::traj_row_data const traj_row, float const torus, int const pid) const;
      
      
      /// **Method**: Examines Theta Phi fiducial cut for pass1
      /// @param traj_row data struct of the particle in REC::Traj
      /// @param torus toroidal magnetic field sign
      /// @param pid pid of the particle
      /// @returns `true` if passes fiducial cuts
      bool DC_fiducial_cut_theta_phi_pass1(FiducialFilter::traj_row_data const traj_row, float const torus, int const pid) const;
      
      /// `hipo::banklist` 
      hipo::banklist::size_type b_particle;
      hipo::banklist::size_type b_traj;
      hipo::banklist::size_type b_cal;
      hipo::banklist::size_type b_config;
      
      /// Pass Reconstruction
      int o_pass = 1;

      /// ECAL cut level
      std::string o_ecal_cut_level;
  };

}
