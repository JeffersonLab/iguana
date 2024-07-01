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
  class LeptonIDFilter : public Algorithm
  {

      DEFINE_IGUANA_ALGORITHM(LeptonIDFilter, clas12::LeptonIDFilter)

    public:

      void Start(hipo::banklist& banks) override;
      void Run(hipo::banklist& banks) const override;
      void Stop() override;

      /// **FindLepton function**: returns the pindex of the lepton
      /// @param particle_bank the particle bank
      /// @returns pindex of the lepton, -1 if there is no lepton
      int FindLepton(hipo::bank const& particle_bank) const;

    
      /// **GetLeptonIDVariables function**: Using the pindex retrieves the necessary variables from banks
      /// @param plepton pindex of the lepton
      /// @param particle_bank the particle bank
      /// @param calorimeter_bank the calorimeter bank
      /// @returns LeptonIDVars, the variables required for identification
      LeptonIDVars GetLeptonIDVariables(int const plepton, hipo::bank const& particle_bank, hipo::bank const& calorimeter_bank) const;


      /// **CalculateScore function**: Using the LeptonIDVars variables calculate the score
      /// @param lepton_vars LeptonIDVars variables
      /// @returns double, the score
      double CalculateScore(LeptonIDVars lepton_vars) const;

      /// **Filter function**: Returns true if the particle passed the cut 
      /// @param score the score obtained from the CalculateScore function
      /// @returns bool, true if score>=cut, false otherwise
      bool Filter(double score) const;

    
    private:

      /// `hipo::banklist` index for the particle bank (as an example)
      hipo::banklist::size_type b_particle;
      hipo::banklist::size_type b_calorimeter;

      /// Example integer configuration option
      int o_pid;
      std::string o_weightfile;
      std::string o_weightfile_fullpath;
      double o_cut;
  };

}
