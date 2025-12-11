#pragma once

#include "iguana/algorithms/Algorithm.h"
#include <TMVA/Reader.h>

/// Struct to store variables
struct LeptonIDVars {
    /// @brief momentum
    double P;
    /// @brief Theta angle
    double Theta;
    /// @brief Phi angle
    double Phi;
    /// @brief Sampling fraction on the PCAL
    double SFpcal;
    /// @brief Sampling fraction on the ECIN
    double SFecin;
    /// @brief Sampling fraction on the ECOUT
    double SFecout;
    /// @brief Second-momenta of PCAL
    double m2pcal;
    /// @brief Second-momenta of ECIN
    double m2ecin;
    /// @brief Second-momenta of ECOUT
    double m2ecout;
    /// @brief Score
    double score;
};

namespace iguana::clas12 {
  ///
  /// @brief_algo Filter the leptons from the pion contamination using TMVA models
  ///
  /// For each lepton, either positron or electron, it takes some variables from `REC::Particle` (P, Theta and Phi) and `REC::Particle` (Sampling fraction and second moments).
  /// Using those variables, it call the TMVA method using the weight file, and it computes a score. By a pplying a cut to the score we can separate leptons from pions.
  ///
  /// @begin_doc_algo{clas12::LeptonIDFilter | Filter}
  /// @input_banks{REC::Particle,REC::Calorimeter}
  /// @end_doc
  ///
  /// @begin_doc_config
  /// @config_param{o_pid | int | PID of the particle; -11 for positrons and 11 for electrons}
  /// @config_param{o_weightfile | std::string | Location of the weight file of the classifier}
  /// @config_param{o_cut | double | Value of the score to apply the cut. The algorith will keep all particles that have a score grater than ths value}
  /// @end_doc
  class LeptonIDFilter : public Algorithm
  {

      DEFINE_IGUANA_ALGORITHM(LeptonIDFilter, clas12::LeptonIDFilter)

    public:
      // Constructor

      void Start(hipo::banklist& banks) override;
      void Run(hipo::banklist& banks) const override;
      void Stop() override;

      void initializeTMVA();

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
      std::unique_ptr<TMVA::Reader> readerTMVA;


      /// Set of variables for the reader
      /// Momentum
      mutable Float_t P;
      /// Theta angle
      mutable Float_t Theta;
      /// Phi angle
      mutable Float_t Phi;
      /// Sampling fraction on the PCAL
      mutable Float_t PCAL;
      /// Sampling fraction on the ECIN
      mutable Float_t ECIN;
      /// Sampling fraction on the ECOUT
      mutable Float_t ECOUT;
      /// Second-momenta of PCAL
      mutable Float_t m2PCAL;
      /// Second-momenta of ECIN
      mutable Float_t m2ECIN;
      /// Second-momenta of ECOUT
      mutable Float_t m2ECOUT;


      /// `hipo::banklist`
      hipo::banklist::size_type b_particle;
      hipo::banklist::size_type b_calorimeter;


      /// pid of the lepton
      int o_pid;
      /// Location of the weight file
      std::string o_weightfile;
      std::string o_weightfile_fullpath;
      /// Value of the cut
      double o_cut;
  };

}
