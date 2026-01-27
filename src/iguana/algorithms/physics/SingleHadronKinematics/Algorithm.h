#pragma once

#include "iguana/algorithms/Algorithm.h"
#include <Math/Vector3D.h>
#include <Math/Vector4D.h>

namespace iguana::physics {

  /// @algo_brief{Calculate semi-inclusive hadron kinematic quantities}
  /// @algo_type_creator
  ///
  /// @begin_doc_config{physics/SingleHadronKinematics}
  /// @config_param{hadron_list | list[int] | calculate kinematics for these hadron PDGs}
  /// @end_doc
  ///
  /// The output bank `%physics::SingleHadronKinematics` will have the same number of rows as the input particle bank
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

    private: // hooks
      void ConfigHook() override;
      void StartHook(hipo::banklist& banks) override;
      bool RunHook(hipo::banklist& banks) const override;

    public:

      /// @run_function
      /// @param [in] particle_bank particle bank (_e.g._, `REC::Particle`)
      /// @param [in] inc_kin_bank `%physics::InclusiveKinematics`, produced by the `physics::InclusiveKinematics` algorithm
      /// @param [out] result_bank `%physics::SingleHadronKinematics`, which will be created
      /// @returns `false` if the input banks do not have enough information, _e.g._, if the inclusive kinematics bank is empty,
      /// or if the created bank is empty
      bool Run(
          hipo::bank const& particle_bank,
          hipo::bank const& inc_kin_bank,
          hipo::bank& result_bank) const;

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
      int i_MX2;
      int i_xF;
      int i_yB;
      int i_phiH;
      int i_xi;

      // config options
      std::string o_particle_bank;
      std::set<int> o_hadron_pdgs;
  };

}
