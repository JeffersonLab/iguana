#pragma once

#include "iguana/algorithms/Algorithm.h"
#include <Math/Vector3D.h>
#include <Math/VectorUtil.h>

namespace iguana::clas12 {

  ///
  /// @brief_algo This is a template algorithm, used as an example showing how to write an algorithm.
  ///
  /// Provide a more detailed description of your algorithm here.
  ///
  /// @begin_doc_algo{Filter}
  /// @input_banks{REC::Particle}
  /// @output_banks{REC::Particle}
  /// @end_doc
  ///
  /// @begin_doc_config
  /// @config_param{exampleInt | int | an example `integer` configuration parameter}
  /// @config_param{exampleDouble | double | an example `double` configuration parameter}
  /// @end_doc
  class PhotonGBTFilter : public Algorithm
  {

      DEFINE_IGUANA_ALGORITHM(PhotonGBTFilter, clas12::PhotonGBTFilter)

    public:

      void Start(hipo::banklist& banks) override;
      void Run(hipo::banklist& banks) const override;
      void Stop() override;

      /// Struct to store calorimeter particle data
      struct calo_row_data {
        double pcal_x = 0;
        double pcal_y = 0;
        double pcal_z = 0;
        double ecin_x = 0;
        double ecin_y = 0;
        double ecin_z = 0;
        double ecout_x = 0;
        double ecout_y = 0;
        double ecout_z = 0;
        double pcal_e = 0;
        double pcal_m2u = 0;
        double pcal_m2v = 0;
      };
      
      /// **Action function**: Classifies the photon for a given event as signal or background
      /// @param particleBank the REC::Particle hipo bank
      /// @param caloBank the REC::Calorimeter hipo bank
      /// @param calo_map the std::map<> of calorimeter data for the event, indexed by pindex
      /// @param row the row corresponding to the photon being classified
      /// @returns `true` if the photon is to be considered signal, otherwise `false`
      bool ClassifyPhoton(hipo::bank const &particleBank, hipo::bank const &caloBank, std::map<int, calo_row_data> calo_map, int const row) const;
          
          
      /// **Method**: Gets calorimeter data for particles in the event
      /// @param bank the bank to get data from
      /// @returns a map with keys as particle indices (pindex) and values as calo_row_data structs
      std::map<int, calo_row_data> GetCaloMap(hipo::bank const &bank) const;
      
      
      /// **Method**: Gets the calorimeter vector for a particle in the event
      /// @param calo_map the map with keys as particle indices (pindex) and values as calo_row_data structs
      /// @param row the row (particle index) to get the calorimeter data for
      /// @returns a ROOT::Math::XYZVector with the coordinates of the particle in the calorimeter
      ROOT::Math::XYZVector GetParticleCaloVector(std::map<int, calo_row_data> const &calo_map, int const row) const;
      
      
      /// **Method**: Gets the mass of a particle given its PID
      /// @param pid the particle ID to get the mass for
      /// @returns the mass of the particle in GeV; returns -1.0 if the PID is not recognized
      double GetMass(int pid) const;
      
      
      /// **Method**: Gets the type of a particle given its PID
      /// @param pid the particle ID to get the type for
      /// @returns an integer representing the type of the particle: 
      /// 0 for electron, 1 for photon, 2 for charged hadron, 3 for neutral hadron; returns -1 if the PID is not recognized
      int GetParticleType(int pid) const;
      
      
    private:

      /// `hipo::banklist` 
      hipo::banklist::size_type b_particle;
      hipo::banklist::size_type b_calorimeter;
      hipo::banklist::size_type b_config; // RUN::config
      
      const std::unordered_map<int, double> mass_map = {
          {11, 0.000511},    // electron
          {22, 0.0},         // photon
          {2212, 0.938272},  // proton
          {-2212, 0.938272}, // antiproton
          {2112, 0.939565},  // neutron
          {-2112, 0.939565}, // antineutron
          {211, 0.139570},   // pi+
          {-211, 0.139570},  // pi-
          {321, 0.493677},   // K+
          {-321, 0.493677}  // K-
      };
      
      
      const std::unordered_map<int, int> type_map = {
          {11, 0},  // electron
          {22, 1},  // photon
          {2212, 2}, // proton (charged hadron)
          {-2212, 2}, // antiproton (charged hadron)
          {211, 2},  // pi+ (charged hadron)
          {-211, 2}, // pi- (charged hadron)
          {321, 2},  // K+ (charged hadron)
          {-321, 2}, // K- (charged hadron)
          {2112, 3}, // neutron (neutral hadron)
          {-2112, 3} // antineutron (neutral hadron)
      };
      
  };

}
