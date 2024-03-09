#pragma once

#include "iguana/algorithms/Algorithm.h"
#include "iguana/algorithms/TypeDefs.h"

namespace iguana::physics {

  /// Set of inclusive kinematics variables
  struct InclusiveKinematicsVars {
      /// @f$q=(q_x,q_y,q_z,q_E)@f$
      vector4_t q;
      /// @f$Q2@f$ (GeV@f$^2@f$)
      double Q2;
      /// @f$x_B@f$
      double x;
      /// @f$y@f$
      double y;
      /// @f$W@f$ (GeV)
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

      /// Find the scattered lepton. Since finding the scattered lepton requires
      /// reading all the particles of an event, there is no **Action function**;
      /// therefore, callers that do not have access to `hipo::bank` objects are
      /// responsible for finding the scattered lepton.
      /// @param particle_bank the particle bank to search
      /// @returns the bank row of the scattered lepton, or `-1` if not found
      int FindScatteredLepton(const hipo::bank& particle_bank) const;

      /// **Action function**: compute kinematics from the scattered lepton.
      /// @param lepton_px scattered lepton momentum component @f$p_x@f$ (GeV)
      /// @param lepton_py scattered lepton momentum component @f$p_y@f$ (GeV)
      /// @param lepton_pz scattered lepton momentum component @f$p_z@f$ (GeV)
      /// @param beam_px beam momentum component @f$p_x@f$ (GeV)
      /// @param beam_py beam momentum component @f$p_y@f$ (GeV)
      /// @param beam_pz beam momentum component @f$p_z@f$ (GeV)
      /// @param target_mass target mass (GeV)
      /// @param lepton_pdg lepton PDG
      /// @returns the reconstructed inclusive kinematics in a `iguana::physics::InclusiveKinematicsVars` instance
      InclusiveKinematicsVars ComputeFromLepton(
          vector_element_t lepton_px,
          vector_element_t lepton_py,
          vector_element_t lepton_pz,
          vector_element_t beam_px,
          vector_element_t beam_py,
          vector_element_t beam_pz,
          double target_mass = particle::mass.at(particle::PDG::proton),
          double lepton_pdg  = particle::PDG::electron) const;

    private:

      hipo::banklist::size_type b_particle;
      hipo::banklist::size_type b_result;

      int o_runnum;
      double o_beam_energy;
      std::vector<double> o_beam_direction;
      std::string o_beam_particle;
      std::string o_target_particle;
      enum method_reconstruction { scattered_lepton };
      enum method_lepton_finder { highest_energy_FD_trigger };
      method_reconstruction o_method_reconstruction;
      method_lepton_finder o_method_lepton_finder;

      struct particle_t {
          int pdg;
          double mass;
          double px;
          double py;
          double pz;
      };
      particle_t m_beam;
      particle_t m_target;
  };

}
