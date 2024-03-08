#pragma once

#include "iguana/algorithms/Algorithm.h"
#include "iguana/algorithms/TypeDefs.h"

namespace iguana::physics {

  /// Set of inclusive kinematics variables
  struct InclusiveKinematicsVars {
    /// @f$q=(q_x,q_y,q_z,q_E)@f$, from which @f$Q^2=|q|^2@f$
    vector4_t q;
    /// @f$x_B@f$
    double xB;
    /// @f$y@f$
    double y;
    /// @f$W@f$
    double W;
    /// @f$\nu@f$
    double nu;
  };

  /// @brief Calculate inclusive kinematics quantities defined in `iguana::physics::InclusiveKinematicsVars`
  class InclusiveKinematics : public Algorithm
  {

      DEFINE_IGUANA_ALGORITHM(InclusiveKinematics, physics::InclusiveKinematics)

    public:

      void Start(hipo::banklist& banks) override;
      void Run(hipo::banklist& banks) const override;
      void Stop() override;

      // Find the scattered lepton. Since finding the scattered lepton requires
      // reading all the particles of an event, there is no **Action function**;
      // therefore, callers that do not have access to `hipo::bank` objects are
      // responsible for finding the scattered lepton.
      // @param particle_bank the particle bank to search
      // @returns the bank row of the scattered lepton, or `-1` if not found
      int FindScatteredLepton(const hipo::bank& particle_bank) const;

      /// **Action function**: compute kinematics from the scattered lepton.
      /// @param lepton_px scattered lepton momentum component @f$p_x@f$ (GeV)
      /// @param lepton_py scattered lepton momentum component @f$p_y@f$ (GeV)
      /// @param lepton_pz scattered lepton momentum component @f$p_z@f$ (GeV)
      /// @param beam_E beam energy (GeV)
      /// @param target_M target mass (GeV)
      /// @param beam_dir_x beam lepton @f$x@f$ direction
      /// @param beam_dir_y beam lepton @f$y@f$ direction
      /// @param beam_dir_z beam lepton @f$z@f$ direction
      /// @param lepton_pdg lepton PDG
      /// @returns the reconstructed inclusive kinematics in a `iguana::physics::InclusiveKinematicsVars` instance
      InclusiveKinematicsVars ComputeFromLepton(
          vector_element_t lepton_px,
          vector_element_t lepton_py,
          vector_element_t lepton_pz,
          double beam_E = 10.6,
          double target_M = particle::mass.at(particle::PDG::proton),
          vector_element_t beam_dir_x = 0.0,
          vector_element_t beam_dir_y = 0.0,
          vector_element_t beam_dir_z = 1.0,
          double lepton_pdg = particle::PDG::electron
          ) const;

    private:

      hipo::banklist::size_type b_particle;

      int o_runnum;
      double o_beam_energy;
      std::string o_beam_particle;
      std::string o_target_particle;
      enum method_reconstruction { scattered_lepton };
      enum method_lepton_finder { highest_energy_FD_trigger };
      method_reconstruction o_method_reconstruction;
      method_lepton_finder o_method_lepton_finder;

      int m_beam_particle_pdg;
      double m_beam_particle_mass;
      double m_target_particle_mass;
  };

}
