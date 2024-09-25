#pragma once

#include "iguana/algorithms/Algorithm.h"
#include "models/RGA_inbending_pass1.cpp"
#include "models/RGA_outbending_pass1.cpp"
#include "models/RGA_inbending_pass2.cpp"
#include "models/RGA_outbending_pass2.cpp"
#include "models/RGC_Summer2022_pass1.cpp"

#include <Math/Vector3D.h>
#include <Math/VectorUtil.h>

namespace iguana::clas12 {

  ///
  /// @brief_algo Filter the `REC::Particle` photons using pretrained GBT models
  ///
  /// For each photon (labeled the photon of interest or POI), we obtain its intrinsic features (energy, angle, pcal edep, etc.) and features corresponding to its nearest neighbors (angle of proximity, energy difference, etc.). This requires the reading of both the REC::Particle and REC::Calorimeter banks. An input std::vector<float> is produced and passed to the pretrained GBT models, which yield a classification score between 0 and 1. An option variable `threshold` then determines the minimum photon `p-value` to survive the cut.
  ///
  /// @begin_doc_algo{clas12::PhotonGBTFilter | Filter}
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

      /// **Method**: Applies forward detector cut using REC::Particle Theta
      /// @param theta lab angle of the particle with respect to the beam direction (radians)
      /// @returns `true` if the particle's theta is within the forward detector coverage, `false` otherwise
      bool ForwardDetectorFilter(float const theta) const;

    private:
      
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
        double ecin_e = 0;
        double ecin_m2u = 0;
        double ecin_m2v = 0;
        double ecout_e = 0;
        double ecout_m2u = 0;
        double ecout_m2v = 0;
      }; 
      
      /// **Method**: Applies pid purity cuts to photons, compatible to how the GBT models are trained
      /// @param E energy of the photon
      /// @param Epcal energy the photon has deposited in the pre-shower calorimeter
      /// @param theta lab angle of the photon with respect to the beam direction (radians)
      /// @returns `true` if the photon passes the pid purity cuts, `false` otherwise
      bool PidPurityPhotonFilter(float const E, float const Epcal, float const theta) const;

      /// **Action function**: Classifies the photon for a given event as signal or background
      /// @param particleBank the REC::Particle hipo bank
      /// @param caloBank the REC::Calorimeter hipo bank
      /// @param calo_map the std::map<> of calorimeter data for the event, indexed by pindex
      /// @param row the row corresponding to the photon being classified
      /// @param runnum the current run number
      /// @returns `true` if the photon is to be considered signal, otherwise `false`
      bool Filter(hipo::bank const &particleBank, hipo::bank const &caloBank, std::map<int, PhotonGBTFilter::calo_row_data> calo_map, int const row, int const runnum) const;
      
      
      /// **Method**: Calls the appropriate CatBoost model for the given run group, classifying the photon of interest
      /// @param input_data the input features of the model
      /// @param runnum the run number associated to the event
      /// @returns `true` if the 
      bool ClassifyPhoton(std::vector<float> const &input_data, int const runnum) const;
      
      
      /// **Method**: Gets calorimeter data for particles in the event
      /// @param bank the bank to get data from
      /// @returns a map with keys as particle indices (pindex) and values as calo_row_data structs
      std::map<int, PhotonGBTFilter::calo_row_data> GetCaloMap(hipo::bank const &bank) const;
      
      
      /// **Method**: Gets the calorimeter vector for a particle in the event
      /// @param crd data struct of a single REC::Calorimeter's row data
      /// @returns a ROOT::Math::XYZVector with the coordinates of the particle in the calorimeter
      ROOT::Math::XYZVector GetParticleCaloVector(PhotonGBTFilter::calo_row_data calo_row) const;
      
      
      /// **Method**: Gets the mass of a particle given its PID
      /// @param pid the particle ID to get the mass for
      /// @returns the mass of the particle in GeV; returns -1.0 if the PID is not recognized
      double GetMass(int pid) const;
      
      
      /// **Method**: Gets the model function for the run number
      /// @param runnum the run of the associated event
      /// @returns GBT function for the run period
      std::function<double(std::vector<float> const &)> getModelFunction(int runnum) const;
      
      /// `hipo::banklist` 
      hipo::banklist::size_type b_particle;
      hipo::banklist::size_type b_calorimeter;
      hipo::banklist::size_type b_config; // RUN::config
      
      /// Threshold value for model predictions
      double o_threshold = 0.78;
      
      /// Integer for the event reconstruction pass
      int o_pass = 1;
    
      /// Map for the GBT Models to use depending on pass and run number
      const std::map<std::tuple<int, int, int>, std::function<double(std::vector<float> const &)>> modelMap = {
            {{5032, 5332, 1}, [](std::vector<float> const &data) { return ApplyCatboostModel_RGA_inbending_pass1(data); }}, // Fall2018 RGA Inbending
            {{5032, 5332, 2}, [](std::vector<float> const &data) { return ApplyCatboostModel_RGA_inbending_pass2(data); }}, // Fall2018 RGA Inbending
            {{5333, 5666, 1}, [](std::vector<float> const &data) { return ApplyCatboostModel_RGA_outbending_pass1(data); }}, // Fall2018 RGA Outbending
            {{5333, 5666, 2}, [](std::vector<float> const &data) { return ApplyCatboostModel_RGA_outbending_pass2(data); }}, // Fall2018 RGA Outbending
            {{6616, 6783, 1}, [](std::vector<float> const &data) { return ApplyCatboostModel_RGA_inbending_pass1(data); }}, // Spring2019 RGA Inbending
            {{6616, 6783, 2}, [](std::vector<float> const &data) { return ApplyCatboostModel_RGA_inbending_pass2(data); }}, // Spring2019 RGA Inbending
            {{6156, 6603, 1}, [](std::vector<float> const &data) { return ApplyCatboostModel_RGA_inbending_pass1(data); }}, // Spring2019 RGB Inbending
            {{6156, 6603, 2}, [](std::vector<float> const &data) { return ApplyCatboostModel_RGA_inbending_pass2(data); }}, // Spring2019 RGB Inbending
            {{11093, 11283, 1}, [](std::vector<float> const &data) { return ApplyCatboostModel_RGA_outbending_pass1(data); }}, // Fall2019 RGB Outbending
            {{11093, 11283, 2}, [](std::vector<float> const &data) { return ApplyCatboostModel_RGA_outbending_pass2(data); }}, // Fall2019 RGB Outbending
            {{11284, 11300, 1}, [](std::vector<float> const &data) { return ApplyCatboostModel_RGA_inbending_pass1(data); }}, // Fall2019 RGB BAND Inbending
            {{11284, 11300, 2}, [](std::vector<float> const &data) { return ApplyCatboostModel_RGA_inbending_pass2(data); }}, // Fall2019 RGB BAND Inbending
            {{11323, 11571, 1}, [](std::vector<float> const &data) { return ApplyCatboostModel_RGA_inbending_pass1(data); }}, // Spring2020 RGB Inbending
            {{11323, 11571, 2}, [](std::vector<float> const &data) { return ApplyCatboostModel_RGA_inbending_pass2(data); }}, // Spring2020 RGB Inbending
            {{16042, 16772, 1}, [](std::vector<float> const &data) { return ApplyCatboostModel_RGC_Summer2022_pass1(data); }}, // Summer2022 RGC Inbending
            {{16042, 16772, 2}, [](std::vector<float> const &data) { return ApplyCatboostModel_RGC_Summer2022_pass1(data); }} // Summer2022 RGC Inbending (no pass2 currently)
       };
  };
    
}
