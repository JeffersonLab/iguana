#pragma once

#include "iguana/algorithms/Algorithm.h"
#include "iguana/algorithms/physics/Tools.h"
#include <Math/Vector3D.h>
#include <Math/Vector4D.h>

namespace iguana::physics {

  /// @brief_algo Calculate semi-inclusive dihadron kinematic quantities defined in `iguana::physics::DihadronKinematicsVars`
  ///
  /// @begin_doc_algo{physics::DihadronKinematics | Creator}
  /// @input_banks{REC::Particle, %physics::InclusiveKinematics}
  /// @output_banks{%physics::DihadronKinematics}
  /// @end_doc
  ///
  /// @begin_doc_config{physics/DihadronKinematics}
  /// @config_param{hadron_a_list | list[int] | list of "hadron A" PDGs}
  /// @config_param{hadron_b_list | list[int] | list of "hadron B" PDGs}
  /// @config_param{phi_r_method | string | method used to calculate @latex{\phi_R} (see section "phiR calculation methods" below)}
  /// @config_param{theta_method | string | method used to calculate @latex{\theta} (see section "theta calculation methods" below)}
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
  /// @par phiR calculation methods
  /// - `"RT_via_covariant_kT"`: use @latex{R_T} computed via covariant @latex{k_T} formula
  ///
  /// @par theta calculation methods
  /// - `"hadron_a"`: use hadron A's "decay angle" in the dihadron rest frame
  ///
  /// @creator_note
  class DihadronKinematics : public Algorithm
  {

      DEFINE_IGUANA_ALGORITHM(DihadronKinematics, physics::DihadronKinematics)

    public:

      void Start(hipo::banklist& banks) override;
      void Run(hipo::banklist& banks) const override;
      void Stop() override;

      /// @brief form dihadrons by pairing hadrons
      /// @param particle_bank the particle bank (`REC::Particle`)
      /// @returns a list of pairs of hadron rows
      std::vector<std::pair<int,int>> PairHadrons(hipo::bank const& particle_bank) const;

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
      int i_PhPerp;
      int i_MX2;
      int i_xF;
      int i_yB;
      int i_phiH;
      int i_phiR;
      int i_theta;

      // config options
      std::set<int> o_hadron_a_pdgs;
      std::set<int> o_hadron_b_pdgs;
      std::string o_phi_r_method;
      std::string o_theta_method;
      enum {e_RT_via_covariant_kT} m_phi_r_method;
      enum {e_hadron_a} m_theta_method;

      // storage for a single hadron
      struct Hadron {
        int row;
        int pdg;
        ROOT::Math::PxPyPzMVector p;
        double z;
        std::optional<ROOT::Math::XYZVector> p_perp;
      };

  };

}
