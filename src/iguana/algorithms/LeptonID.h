#pragma once

#include "iguana/algorithms/Algorithm.h"

namespace iguana::clas12 {

  /// Set of inclusive kinematics variables
  struct LeptonIDVars {
      double P;
      double Theta;
      double Phi;
      double SFpcal;
      double SFecin;
      double SFecout;
      double m2pcal;
      double m2ecin;
      double m2ecout;
      double score;
  };

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
  class LeptonID : public Algorithm
  {

      DEFINE_IGUANA_ALGORITHM(LeptonID, clas12::LeptonID)

    public:

      void Start(hipo::banklist& banks) override;
      void Run(hipo::banklist& banks) const override;
      void Stop() override;

      /// **FindLepton function**: returns the pindex of the lepton
      /// @param particle_bank the particle bank
      /// @returns pindex of the lepton, -1 if there is no lepton
      int FindLepton(hipo::bank const& particle_bank) const;

      /// **CalculateScore function**: Using the pindex retrieves the necessary variables from banks
      ///to do the Lepton ID
      /// @param plepton pindex of the lepton
      /// @param particle_bank the particle bank
      /// @param calorimeter_bank the calorimeter bank
      /// @returns LeptonIDVars, the variables required for identification
      LeptonIDVars CalculateScore(int const plepton, hipo::bank const& particle_bank, hipo::bank const& calorimeter_bank) const;

    private:

      /// `hipo::banklist` index for the particle bank (as an example)
      hipo::banklist::size_type b_particle;
      hipo::banklist::size_type b_calorimeter;

      /// Example integer configuration option
      int o_pid;
      /// Example double configuration option
      //double o_exampleDouble;
  };

}
