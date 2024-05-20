#pragma once

#include "iguana/algorithms/machine_learning/photon_gbt/RGA_inbending.cpp"
#include "iguana/algorithms/machine_learning/photon_gbt/RGA_outbending.cpp"
#include "iguana/algorithms/machine_learning/photon_gbt/RGC_Summer2022.cpp"
#include "iguana/algorithms/Algorithm.h"
#include "iguana/algorithms/TypeDefs.h"
#include <Math/Vector3D.h>
#include <Math/VectorUtil.h>

namespace iguana::clas12 {

  ///
  /// @brief_algo Filter the `REC::Particle` photons using pretrained GBT models
  ///
  /// For each photon (labeled the photon of interest or POI), we obtain its intrinsic features (energy, angle, pcal edep, etc.) and features corresponding to its nearest neighbors (angle of proximity, energy difference, etc.). This requires the reading of both the REC::Particle and REC::Calorimeter banks. An input std::vector<float> is produced and passed to the pretrained GBT models, which yield a classification score between 0 and 1. An option variable `threshold` then determines the minimum photon `p-value` to survive the cut.
  ///
  /// @begin_doc_algo{Filter}
  /// @input_banks{REC::Particle, REC::Calorimeter, RUN::config}
  /// @output_banks{REC::Particle}
  /// @end_doc
  ///
  /// @begin_doc_config
  /// @config_param{pass | int | cook type}
  /// @config_param{threshold | double | minimum value to qualify a photon as "true"}
  /// @end_doc
  class PhotonGBTFilter : public Algorithm
  {

      DEFINE_IGUANA_ALGORITHM(PhotonGBTFilter, clas12::PhotonGBTFilter)

    public:

      void Start(hipo::banklist& banks) override;
      void Run(hipo::banklist& banks) const override;
      void Stop() override;
      
      /// **Method**: Applies pid purity cuts to photons, compatible to how the GBT models are trained
      /// @param E energy of the photon
      /// @param Epcal energy the photon has deposited in the pre-shower calorimeter
      /// @param theta lab angle of the photon with respect to the beam direction (radians)
      /// @returns `true` if the photon passes the pid purity cuts, `false` otherwise
      bool PidPurityPhotonFilter(float const E, float const Epcal, float const theta) const;
      
      
      /// **Method**: Applies forward detector cut using REC::Particle Theta
      /// @param theta lab angle of the particle with respect to the beam direction (radians)
      /// @returns `true` if the particle's theta is within the forward detector coverage, `false` otherwise
      bool ForwardDetectorFilter(float const theta) const;
      
      /// **Action function**: Classifies the photon for a given event as signal or background
      /// @param particleBank the REC::Particle hipo bank
      /// @param caloBank the REC::Calorimeter hipo bank
      /// @param calo_map the std::map<> of calorimeter data for the event, indexed by pindex
      /// @param row the row corresponding to the photon being classified
      /// @param runnum the current run number
      /// @returns `true` if the photon is to be considered signal, otherwise `false`
      bool Filter(hipo::bank const &particleBank, hipo::bank const &caloBank, std::map<int, calo_row_data> calo_map, int const row, int const runnum) const;
      
      
      /// **Method**: Calls the appropriate CatBoost model for the given run group, classifying the photon of interest
      /// @param input_data the input features of the model
      /// @param runnum the run number associated to the event
      /// @returns `true` if the 
      bool ClassifyPhoton(std::vector<float> const &input_data, int const runnum) const;
      
      
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
      
      /// Threshold value for model predictions
      double o_threshold = 0.78;
      
      /// Integer for the event reconstruction pass
      int o_pass = 1;
      
      /// Particle Type Map
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

