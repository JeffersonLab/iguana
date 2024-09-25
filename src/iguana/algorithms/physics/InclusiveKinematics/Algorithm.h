#pragma once

#include "iguana/algorithms/Algorithm.h"
#include "iguana/algorithms/TypeDefs.h"
#include "iguana/services/ConcurrentParam.h"

namespace iguana::physics {

  /// Set of inclusive kinematics variables
  struct InclusiveKinematicsVars {
      /// @f$x@f$-component of virtual photon momentum @f$q@f$
      vector_element_t qx;
      /// @f$y@f$-component of virtual photon momentum @f$q@f$
      vector_element_t qy;
      /// @f$z@f$-component of virtual photon momentum @f$q@f$
      vector_element_t qz;
      /// @f$E@f$-component of virtual photon momentum @f$q@f$
      vector_element_t qE;
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

  /// @brief_algo Calculate inclusive kinematics quantities defined in `iguana::physics::InclusiveKinematicsVars`
  ///
  /// @begin_doc_algo{physics::InclusiveKinematics | Creator}
  /// @input_banks{REC::Particle, RUN::config}
  /// @output_banks{%physics::InclusiveKinematics}
  /// @end_doc
  ///
  /// @begin_doc_config
  /// @config_param{initial_state | dict | description of the initial state}
  /// @config_param{method:reconstruction | string | kinematics reconstruction method; only `scattered_lepton` is available}
  /// @config_param{method:lepton_finder | string | algorithm to find the scattered lepton; only `highest_energy_FD_trigger` is available}
  /// @end_doc
  class InclusiveKinematics : public Algorithm
  {

      DEFINE_IGUANA_ALGORITHM(InclusiveKinematics, physics::InclusiveKinematics)

    public:

      void Start(hipo::banklist& banks) override;
      void Run(hipo::banklist& banks) const override;
      void Stop() override;

      /// @action_function{reload} prepare the event
      /// @when_to_call{for each event}
      /// @param runnum the run number
      /// @returns the key to be used in `::ComputeFromLepton`
      concurrent_key_t PrepareEvent(int const runnum) const;

      /// @action_function{scalar creator} compute kinematics from the scattered lepton.
      /// @param lepton_px scattered lepton momentum component @f$p_x@f$ (GeV)
      /// @param lepton_py scattered lepton momentum component @f$p_y@f$ (GeV)
      /// @param lepton_pz scattered lepton momentum component @f$p_z@f$ (GeV)
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
      /// @returns the bank row of the scattered lepton, or `-1` if not found
      int FindScatteredLepton(hipo::bank const& particle_bank, concurrent_key_t const key) const;

      void Reload(int const runnum, concurrent_key_t key) const;

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

      // config options
      mutable std::unique_ptr<ConcurrentParam<int>> o_runnum;
      mutable std::unique_ptr<ConcurrentParam<std::vector<double>>> o_target_PxPyPzM;
      mutable std::unique_ptr<ConcurrentParam<std::vector<double>>> o_beam_PxPyPzM;
      double o_beam_mass; // unlikely to change
      int o_beam_pdg; // unlikely to change

      enum method_reconstruction { scattered_lepton };
      enum method_lepton_finder { highest_energy_FD_trigger };
      method_reconstruction o_method_reconstruction;
      method_lepton_finder o_method_lepton_finder;
  };

}
