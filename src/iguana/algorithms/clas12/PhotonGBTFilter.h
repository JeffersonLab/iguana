#pragma once

#include "iguana/algorithms/Algorithm.h"

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

      /// **Action function**: checks if the PDG `pid` is positive;
      /// this is an example action function, please replace it with your own
      /// @param pid the particle PDG to check
      /// @returns `true` if `pid` is positive
      bool Filter(int const pid) const;


      /// Struct to store calorimeter particle data
      struct calo_particle_data {
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
      
      /// **Method**: Gets calorimeter data for particles in the event
      /// @param bank the bank to get data from
      /// @returns a map with keys as particle indices (pindex) and values as calo_particle_data structs
      std::map<int, calo_particle_data> GetCaloData(hipo::bank const &bank) const;

    private:

      /// `hipo::banklist` 
      hipo::banklist::size_type b_particle;
      hipo::banklist::size_type b_calorimeter;
      hipo::banklist::size_type b_config; // RUN::config
      
      
  };

}
