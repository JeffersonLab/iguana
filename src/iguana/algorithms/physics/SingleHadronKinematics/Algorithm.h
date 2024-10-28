#pragma once

#include "iguana/algorithms/Algorithm.h"
#include <Math/Vector3D.h>
#include <Math/Vector4D.h>

namespace iguana::physics {

  /// Set of hadron kinematics variables
  struct SingleHadronKinematicsVars {
    /// @brief `REC::Particle` row (`pindex`) of the hadron
    int pindex;
    /// @brief PDG code of the hadron
    int pdg;
    /// @brief @latex{z}: Momentum fraction of the fragmenting parton carried by the hadron
    double z;
    /// @brief @latex{P_h^\perp}: transverse momentum of the hadron in the @latex{\perp}-frame (transverse to @latex{\vec{q}})
    double PhPerp;
    /// @brief @latex{M_X(ehX)}: Missing mass of the hadron
    double MX;
    /// @brief @latex{x_F}: Feynman-x of the hadron
    double xF;
    /// @brief @latex{\phi_h}: @latex{q}-azimuthal angle between the lepton-scattering plane and the @latex{\vec{q}\times\vec{P}_h} plane;
    /// if the value is `tools::UNDEF`, the calculation failed
    double phiH;
    /// @brief @latex{\xi_h}: Longitudinal momentum fraction of the nucleon carried by the hadron
    double xi;
  };

  /// @brief_algo Calculate semi-inclusive hadron kinematic quantities defined in `iguana::physics::SingleHadronKinematicsVars`
  ///
  /// @begin_doc_algo{physics::SingleHadronKinematics | Creator}
  /// @input_banks{REC::Particle, %physics::InclusiveKinematics}
  /// @output_banks{%physics::SingleHadronKinematics}
  /// @end_doc
  ///
  /// @begin_doc_config
  /// @config_param{hadron_list | list[int] | list of hadron PDGs}
  /// @end_doc
  ///
  /// The output bank `%physics::SingleHadronKinematics` will have the same number of rows as the input particle bank `REC::Particle`
  /// - we want the output bank to have the same number of rows and ordering as the input
  ///   particle bank, so that banks which reference the input particle bank's rows (usually via `pindex`) can be used to
  ///   reference the output bank's rows too
  /// - rows of the input particle bank which were filtered upstream will also be filtered out here, and all the values of the
  ///   corresponding row in the output bank will be zeroed, since no calculations are performed for
  ///   those particles
  /// - particles which are not listed in the configuration parameter `hadron_list` will also be filtered out and zeroed
  class SingleHadronKinematics : public Algorithm
  {

      DEFINE_IGUANA_ALGORITHM(SingleHadronKinematics, physics::SingleHadronKinematics)

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
      int i_pindex;
      int i_pdg;
      int i_z;
      int i_PhPerp;
      int i_MX;
      int i_xF;
      int i_phiH;
      int i_xi;

      // config options
      std::set<int> o_hadron_pdgs;

  };

}
