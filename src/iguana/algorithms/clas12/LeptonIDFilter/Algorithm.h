#pragma once

#include "iguana/algorithms/Algorithm.h"
#include "iguana/services/ConcurrentParam.h"
#include <TMVA/Reader.h>

/// Struct to store variables
struct LeptonIDVars {
    /// @brief momentum
    Double_t P;
    /// @brief Theta angle
    Double_t Theta;
    /// @brief Phi angle
    Double_t Phi;
    /// @brief Sampling fraction on the PCAL
    Double_t SFpcal;
    /// @brief Sampling fraction on the ECIN
    Double_t SFecin;
    /// @brief Sampling fraction on the ECOUT
    Double_t SFecout;
    /// @brief Second-momenta of PCAL
    Double_t m2pcal;
    /// @brief Second-momenta of ECIN
    Double_t m2ecin;
    /// @brief Second-momenta of ECOUT
    Double_t m2ecout;
    /// @brief Score
    Double_t score;
    /// @brief PDG code
    Int_t pid;

    /// @return list of variable values, to pass to `TMVA::Reader::EvaluateMVA`
    std::vector<Double_t> GetValues()
    {
      return {
          // NOTE: order must be consistent with `names`
          P,
          Theta,
          Phi,
          SFpcal,
          SFecin,
          SFecout,
          m2pcal,
          m2ecin,
          m2ecout,
      };
    }

    /// list of variable names, to pass to `TMVA::Reader` constructor
    inline static std::vector<std::string> names = {
        // NOTE: order must be consistent with `GetValues`
        "P",
        "Theta",
        "Phi",
        "SFPCAL",
        "SFECIN",
        "SFECOUT",
        "m2PCAL",
        "m2ECIN",
        "m2ECOUT",
    };
};

namespace iguana::clas12 {
  /// @algo_brief{Filter the leptons from the pion contamination using TMVA models}
  /// @algo_type_filter
  ///
  /// For each lepton, either positron or electron, it takes some variables from `REC::Particle` (P, Theta and Phi) and `REC::Particle` (Sampling fraction and second moments).
  /// Using those variables, it call the TMVA method using the weight file, and it computes a score. By a pplying a cut to the score we can separate leptons from pions.
  ///
  /// @begin_doc_config{clas12/LeptonIDFilter}
  /// @end_doc
  class LeptonIDFilter : public Algorithm
  {

      DEFINE_IGUANA_ALGORITHM(LeptonIDFilter, clas12::LeptonIDFilter)

    private: // hooks
      void ConfigHook() override;
      void StartHook(hipo::banklist& banks) override;
      bool RunHook(hipo::banklist& banks) const override;

    public:

      /// @run_function
      /// @param [in,out] particleBank particle bank (_viz._, `REC::Particle`), which will be filtered
      /// @param [in] calorimeterBank `REC::Calorimeter` bank
      /// @param [in] configBank `RUN::config` bank
      /// @returns `false` if all particles are filtered out
      bool Run(hipo::bank& particleBank, hipo::bank const& calorimeterBank, hipo::bank const& configBank) const;

      /// @action_function{reload} prepare the event
      /// @when_to_call{for each event}
      /// @param runnum the run number
      /// @returns the key to be used in `::Filter`
      concurrent_key_t PrepareEvent(int const runnum) const;

      /// @brief Using the LeptonIDVars, variables calculate the score
      /// @param lepton_vars LeptonIDVars variables
      /// @param key the return value of `::PrepareEvent`
      /// @returns double, the score
      double CalculateScore(LeptonIDVars lepton_vars, concurrent_key_t const key) const;

      /// @brief Returns true if the particle passed the cut
      /// @param score the score obtained from the CalculateScore function
      /// @returns bool, true if score>=cut, false otherwise
      bool Filter(double score) const;

    private:

      // Reload function
      void Reload(int const runnum, concurrent_key_t key) const;

      /// @brief Using the pindex, retrieves the necessary variables from banks
      /// @param plepton pindex of the lepton
      /// @param pdg the PDG code of the lepton
      /// @param particle_bank the particle bank
      /// @param calorimeter_bank the calorimeter bank
      /// @returns LeptonIDVars, the variables required for identification
      LeptonIDVars GetLeptonIDVariables(int const plepton, int const pdg, hipo::bank const& particle_bank, hipo::bank const& calorimeter_bank) const;

      /// TMVA reader
      std::unique_ptr<TMVA::Reader> readerTMVA;

      /// `hipo::banklist`
      hipo::banklist::size_type b_particle;
      hipo::banklist::size_type b_calorimeter;
      hipo::banklist::size_type b_config;

      // config options
      std::set<int> o_pids;
      mutable std::unique_ptr<ConcurrentParam<int>> o_runnum;
      mutable std::unique_ptr<ConcurrentParam<std::string>> o_weightfile_electron;
      mutable std::unique_ptr<ConcurrentParam<std::string>> o_weightfile_positron;
      double o_cut;
      std::string o_tmva_reader_options;
      std::string o_particle_bank;
  };

}
