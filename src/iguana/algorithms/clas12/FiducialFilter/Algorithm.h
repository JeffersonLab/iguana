#pragma once

#include "iguana/algorithms/Algorithm.h"

namespace iguana::clas12 {

  /// @brief_algo Filter the `REC::Particle` bank by applying DC (drift chamber) fiducial cuts
  ///
  /// @begin_doc_algo{clas12::FiducialFilter | Filter}
  /// @input_banks{REC::Particle, REC::Traj, RUN::config}
  /// @output_banks{REC::Particle}
  /// @end_doc
  ///
  /// @begin_doc_config
  /// @config_param{pass | int | cook type to use for assigning fiducial cuts}
  /// @end_doc
  class FiducialFilter : public Algorithm
  {

      DEFINE_IGUANA_ALGORITHM(FiducialFilter, clas12::FiducialFilter)

    public:

      void Start(hipo::banklist& banks) override;
      void Run(hipo::banklist& banks) const override;
      void Stop() override;

      
      
    private:
      
     /// Struct to store trajectory particle data
     struct traj_row_data {
        double x1 = -999;
        double x2 = -999;
        double x3 = -999;
        double y1 = -999;
        double y2 = -999;
        double y3 = -999;
        double z1 = -999;
        double z2 = -999;
        double z3 = -999;
        int sector= 0;
      };    
    
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
          
          
      /// **Method**: Gets trajectory data for particles in the event
      /// @param bank the bank to get data from
      /// @returns a map with keys as particle indices (pindex) and values as traj_row_data structs
      std::map<int, FiducialFilter::traj_row_data> GetTrajMap(hipo::bank const &bank) const;
      
      
      /// **Method**: Gets trajectory data for particles in the event
      /// @param x Drift Chamber x coord
      /// @param y Drift Chamber y coord
      /// @param z Drift Chamber z coord
      /// @returns sector number in DC
      int determineSectorDC(float x, float y, float z) const;
      
      /// `hipo::banklist` 
      hipo::banklist::size_type b_particle;
      hipo::banklist::size_type b_traj;
      hipo::banklist::size_type b_config;
      
      /// Pi
      const double PI = M_PI;
      
      /// Pass Reconstruction
      int o_pass = 1;
      
  };

}
