#pragma once

#include "iguana/algorithms/Algorithm.h"
#include <Math/Vector3D.h>
#include <Math/Vector4D.h>

namespace iguana::physics {

  /// @brief_algo Calculate semi-inclusive hadron kinematic quantities
  ///
  /// @begin_doc_algo{physics::SingleHadronKinematics | Creator}
  /// @input_banks{REC::Particle, %physics::InclusiveKinematics}
  /// @output_banks{%physics::SingleHadronKinematics}
  /// @end_doc
  ///
  /// @begin_doc_config{physics/SingleHadronKinematics}
  /// @config_param{hadron_list | list[int] | calculate kinematics for these hadron PDGs}
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
  ///
  /// @creator_note
  class SingleHadronKinematics : public Algorithm
  {

      DEFINE_IGUANA_ALGORITHM(SingleHadronKinematics, physics::SingleHadronKinematics)

    public:

      void Start(hipo::banklist& banks) override;
      bool Run(hipo::banklist& banks) const override;
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
      int i_MX2;
      int i_xF;
      int i_yB;
      int i_phiH;
      int i_xi;

      // config options
      std::set<int> o_hadron_pdgs;

  };

}
