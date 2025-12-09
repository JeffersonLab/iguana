#pragma once

#include "iguana/algorithms/Algorithm.h"
#include "iguana/algorithms/TypeDefs.h"
#include "iguana/services/ConcurrentParam.h"

namespace iguana::physics {

  /// @algo_brief{Calculate inclusive kinematics quantities}
  /// @algo_type_creator
  /// @begin_doc_config{physics/InclusiveKinematics}
  /// @config_param{beam_direction | list[double] | beam direction vector}
  /// @config_param{target_particle | string | target particle}
  /// @config_param{beam_particle | string | beam particle}
  /// @config_param{reconstruction | string | kinematics reconstruction method; only `scattered_lepton` is available at this time}
  /// @config_param{lepton_finder | string | algorithm to find the scattered lepton}
  /// @end_doc
  /// @rcdb_note
  class InclusiveKinematics : public Algorithm
  {

      DEFINE_IGUANA_ALGORITHM(InclusiveKinematics, physics::InclusiveKinematics)

    public:

      void Start(hipo::banklist& banks) override;
      bool Run(hipo::banklist& banks) const override;
      void Stop() override;

      /// @run_function
      /// @param [in] particle_bank particle bank (_e.g._, `REC::Particle`)
      /// @param [in] config_bank `RUN::config`
      /// @param [out] result_bank `%physics::InclusiveKinematics`, which will be created
      /// @returns `true` if the kinematics were calculated; _e.g._, if the calculations are performed using
      /// the scattered lepton, and no scattered lepton was found, `false` will be returned
      bool Run(
          hipo::bank const& particle_bank,
          hipo::bank const& config_bank,
          hipo::bank& result_bank) const;

      /// @action_function{reload} prepare the event
      /// @when_to_call{for each event}
      /// @param runnum the run number
      /// @param beam_energy the beam energy; if negative (the default), RCDB will be used to get the beam energy from `runnum`
      /// @returns the key to be used in `::ComputeFromLepton`
      concurrent_key_t PrepareEvent(int const runnum, double const beam_energy = -1) const;

      /// @action_function{scalar creator} compute kinematics from the scattered lepton.
      /// @param lepton_px scattered lepton momentum component @latex{p_x} (GeV)
      /// @param lepton_py scattered lepton momentum component @latex{p_y} (GeV)
      /// @param lepton_pz scattered lepton momentum component @latex{p_z} (GeV)
      /// @param key the return value of `::PrepareEvent`
      /// @returns the reconstructed inclusive kinematics in a `iguana::physics::InclusiveKinematicsVars` instance
      InclusiveKinematicsVars ComputeFromLepton(
          vector_element_t const lepton_px,
          vector_element_t const lepton_py,
          vector_element_t const lepton_pz,
          concurrent_key_t const key) const;

    private:

      /// FIXME: this could be changed to a vector action function
      /// Find the scattered lepton. Since finding the scattered lepton requires
      /// reading all the particles of an event, there is no **Action function**;
      /// therefore, callers that do not have access to `hipo::bank` objects are
      /// responsible for finding the scattered lepton.
      /// @param particle_bank the particle bank to search
      /// @param key the return value of `::PrepareEvent`
      /// @returns the bank row of the scattered lepton, if found
      std::optional<int> const FindScatteredLepton(hipo::bank const& particle_bank, concurrent_key_t const key) const;

      void Reload(int const runnum, double const user_beam_energy, concurrent_key_t key) const;

      // banklist indices
      hipo::banklist::size_type b_particle;
      hipo::banklist::size_type b_config;
      hipo::banklist::size_type b_result;

      // `b_result` bank item indices
      int i_pindex;
      int i_Q2;
      int i_x;
      int i_y;
      int i_W;
      int i_nu;
      int i_qx;
      int i_qy;
      int i_qz;
      int i_qE;
      int i_beamPz;
      int i_targetM;

      // config options
      std::string o_particle_bank;
      mutable std::unique_ptr<ConcurrentParam<int>> o_runnum;
      mutable std::unique_ptr<ConcurrentParam<std::vector<double>>> o_target_PxPyPzM;
      mutable std::unique_ptr<ConcurrentParam<std::vector<double>>> o_beam_PxPyPzM;
      double o_beam_mass; // unlikely to change
      int o_beam_pdg; // unlikely to change
      double o_override_beam_energy;
      double o_theta_between_FD_and_FT;

      enum method_reconstruction {
        scattered_lepton
      };
      enum method_lepton_finder {
        highest_energy_FD_trigger,
        lund_beam_daughter,
      };
      method_reconstruction o_method_reconstruction;
      method_lepton_finder o_method_lepton_finder;
  };

}
