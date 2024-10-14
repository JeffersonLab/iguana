#pragma once

#include "iguana/algorithms/Algorithm.h"

namespace iguana::physics {

  /// Set of dihadron kinematics variables
  struct DihadronKinematicsVars {
    /// `REC::Particle` row (`pindex`) of hadron A
    int pindex_a;
    /// `REC::Particle` row (`pindex`) of hadron B
    int pindex_b;
    /// PDG code of hadron A
    int pdg_a;
    /// PDG code of hadron B
    int pdg_b;
    /// @latex{M_h}: Invariant mass of the dihadron
    double Mh;
    /// @latex{z}: Fraction of energy of fragmenting parton carried by the hadron
    double z;
    /// @latex{M_X(ehhX)}: Missing mass of the dihadron
    double MX;
    /// @latex{x_F}: Feynman-x of the dihadron
    double xF;
    /// @latex{\phi_h}: @latex{q}-azimuthal angle between the lepton-scattering plane and the @latex{\vec{q}\times\vec{P}_h} plane
    double phiH;
    /// @latex{\phi_R}: @latex{q}-azimuthal angle between the lepton-scattering plane and dihadron plane
    double phiR;
  };

  /// @brief_algo Calculate semi-inclusive dihadron kinematic quantities defined in `iguana::physics::DihadronKinematicsVars`
  ///
  /// @begin_doc_algo{physics::DihadronKinematics | Creator}
  /// @input_banks{REC::Particle}
  /// @output_banks{%physics::DihadronKinematics}
  /// @end_doc
  ///
  /// @begin_doc_config
  /// @config_param{hadron_a_list | list[int] | list of "hadron A" PDGs}
  /// @config_param{hadron_b_list | list[int] | list of "hadron B" PDGs}
  /// @config_param{phi_r_method | string | method used to calculate @latex{\phi_R} (see below)}
  /// @end_doc
  ///
  /// Dihadron PDGs will be formed from pairs from `hadron_a_list` and `hadron_b_list`. For example,
  /// if you define:
  /// ```yaml
  /// hadron_a_list: [ 211 ]
  /// hadron_b_list: [ -211, 2212 ]
  /// ```
  /// then the algorithm will calculate kinematics for @latex{\pi^+\pi^-} and @latex{\pi^+p} dihadrons; hadron A
  /// is the @latex{\pi^+} for both of these, whereas hadron B is the @latex{\pi^-} for the former and the proton
  /// for the latter.
  ///
  /// @par @latex{\phi_R} calculation methods
  /// - `"RT_via_covariant_kT"`: use @latex{R_T} computed via covariant @latex{k_T} formula
  class DihadronKinematics : public Algorithm
  {

      DEFINE_IGUANA_ALGORITHM(DihadronKinematics, physics::DihadronKinematics)

    public:

      void Start(hipo::banklist& banks) override;
      void Run(hipo::banklist& banks) const override;
      void Stop() override;

    private:

      // banklist indices
      hipo::banklist::size_type b_particle;
      hipo::banklist::size_type b_inc_kin;
      hipo::banklist::size_type b_result;

      // `b_result` bank item indices
      int i_pindex_a;
      int i_pindex_b;
      int i_pdg_a;
      int i_pdg_b;
      int i_Mh;
      int i_z;
      int i_MX;
      int i_xF;
      int i_phiH;
      int i_phiR;

      // config options
      std::set<int> o_hadron_a_pdgs;
      std::set<int> o_hadron_b_pdgs;
      std::string o_phi_r_method;

  };

}
