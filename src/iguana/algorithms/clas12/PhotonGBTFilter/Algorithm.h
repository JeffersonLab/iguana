#pragma once

#include "iguana/algorithms/Algorithm.h"

#include <Math/Vector3D.h>
#include <Math/VectorUtil.h>

namespace iguana::clas12 {

  ///
  /// @algo_brief{Filter the `REC::Particle` photons using pretrained GBT models}
  /// @algo_type_filter
  ///
  /// For each photon (labeled the photon of interest or POI), we obtain its intrinsic features (energy, angle, pcal edep, etc.) and features corresponding to its nearest neighbors (angle of proximity, energy difference, etc.). This requires the reading of both the REC::Particle and REC::Calorimeter banks. An input std::vector<float> is produced and passed to the pretrained GBT models, which yield a classification score between 0 and 1. An option variable `threshold` then determines the minimum photon `p-value` to survive the cut.
  ///
  /// @begin_doc_config{clas12/PhotonGBTFilter}
  /// @config_param{pass | int | cook type}
  /// @config_param{threshold | double | minimum value to qualify a photon as "true"}
  /// @end_doc
  class PhotonGBTFilter : public Algorithm
  {

      DEFINE_IGUANA_ALGORITHM(PhotonGBTFilter, clas12::PhotonGBTFilter)

    public:

      void Start(hipo::banklist& banks) override;
      bool Run(hipo::banklist& banks) const override;
      void Stop() override;

      /// @run_function
      /// @param [in,out] particleBank `REC::Particle`, which will be filtered
      /// @param [in] caloBank `REC::Calorimeter`
      /// @param [in] configBank `RUN::config`
      /// @returns `false` if all particles are filtered out
      bool Run(
          hipo::bank& particleBank,
          hipo::bank const& caloBank,
          hipo::bank const& configBank) const;

      /// Applies forward detector cut using REC::Particle Theta
      /// @param theta lab angle of the particle with respect to the beam direction (radians)
      /// @returns `true` if the particle's theta is within the forward detector coverage, `false` otherwise
      bool ForwardDetectorFilter(float const theta) const;

    private:

      struct calo_row_data {
          double pcal_x    = 0;
          double pcal_y    = 0;
          double pcal_z    = 0;
          double ecin_x    = 0;
          double ecin_y    = 0;
          double ecin_z    = 0;
          double ecout_x   = 0;
          double ecout_y   = 0;
          double ecout_z   = 0;
          double pcal_e    = 0;
          double pcal_m2u  = 0;
          double pcal_m2v  = 0;
          double ecin_e    = 0;
          double ecin_m2u  = 0;
          double ecin_m2v  = 0;
          double ecout_e   = 0;
          double ecout_m2u = 0;
          double ecout_m2v = 0;
      };

      /// Applies pid purity cuts to photons, compatible to how the GBT models are trained
      /// @param E energy of the photon
      /// @param Epcal energy the photon has deposited in the pre-shower calorimeter
      /// @param theta lab angle of the photon with respect to the beam direction (radians)
      /// @returns `true` if the photon passes the pid purity cuts, `false` otherwise
      bool PidPurityPhotonFilter(float const E, float const Epcal, float const theta) const;

      /// Classifies the photon for a given event as signal or background
      /// @param particleBank the REC::Particle hipo bank
      /// @param caloBank the REC::Calorimeter hipo bank
      /// @param calo_map the std::map<> of calorimeter data for the event, indexed by pindex
      /// @param row the row corresponding to the photon being classified
      /// @param runnum the current run number
      /// @returns `true` if the photon is to be considered signal, otherwise `false`
      bool Filter(hipo::bank const& particleBank, hipo::bank const& caloBank, std::map<int, PhotonGBTFilter::calo_row_data> calo_map, int const row, int const runnum) const;


      /// Calls the appropriate CatBoost model for the given run group, classifying the photon of interest
      /// @param input_data the input features of the model
      /// @param runnum the run number associated to the event
      /// @returns `true` if the
      bool ClassifyPhoton(std::vector<float> const& input_data, int const runnum) const;


      /// Gets calorimeter data for particles in the event
      /// @param bank the bank to get data from
      /// @returns a map with keys as particle indices (pindex) and values as calo_row_data structs
      std::map<int, PhotonGBTFilter::calo_row_data> GetCaloMap(hipo::bank const& bank) const;


      /// Gets the calorimeter vector for a particle in the event
      /// @param crd data struct of a single REC::Calorimeter's row data
      /// @returns a ROOT::Math::XYZVector with the coordinates of the particle in the calorimeter
      ROOT::Math::XYZVector GetParticleCaloVector(PhotonGBTFilter::calo_row_data calo_row) const;

      /// Gets the model function for the run number
      /// @param runnum the run of the associated event
      /// @returns GBT function for the run period
      std::function<double(std::vector<float> const&)> getModelFunction(int runnum) const;

      /// `hipo::banklist`
      hipo::banklist::size_type b_particle;
      hipo::banklist::size_type b_calorimeter;
      hipo::banklist::size_type b_config; // RUN::config

      /// Threshold value for model predictions
      double o_threshold = 0.78;

      /// Integer for the event reconstruction pass
      int o_pass = 1;

      /// Map for the GBT Models to use depending on pass and run number
      static std::map<std::tuple<int, int, int>, std::function<double(std::vector<float> const&)>> const modelMap;
  };

}
