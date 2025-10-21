#pragma once

#include "iguana/algorithms/Algorithm.h"

namespace iguana::physics {

  /// @algo_brief{Calculate depolarization factors}
  /// @algo_type_creator
  ///
  /// @par References
  /// - https://arxiv.org/pdf/hep-ph/0611265
  /// - https://arxiv.org/pdf/1408.5721
  class Depolarization : public Algorithm
  {

      DEFINE_IGUANA_ALGORITHM(Depolarization, physics::Depolarization)

    public:

      void Start(hipo::banklist& banks) override;
      bool Run(hipo::banklist& banks) const override;
      void Stop() override;

      /// @run_function
      /// @param [in] inc_kin_bank `%physics::InclusiveKinematics`, produced by the `physics::InclusiveKinematics` algorithm
      /// @param [out] result_bank `%physics::Depolarization`, which will be created
      /// @run_function_returns_true
      bool Run(
          hipo::bank const& inc_kin_bank,
          hipo::bank& result_bank) const;

      /// @action_function{scalar creator} compute depolarization factors
      /// @param Q2 @latex{Q^2}, from `iguana::physics::InclusiveKinematics`
      /// @param x Bjorken-@latex{x}, from `iguana::physics::InclusiveKinematics`
      /// @param y @latex{y}, from `iguana::physics::InclusiveKinematics`
      /// @param targetM the target mass (likely the proton mass)
      /// @returns the depolarization factors in a `iguana::physics::DepolarizationVars` instance
      DepolarizationVars Compute(double const Q2, double const x, double const y, double const targetM) const;

    private:

      // banklist indices
      hipo::banklist::size_type b_inc_kin;
      hipo::banklist::size_type b_result;

      // `b_result` bank item indices
      int i_epsilon;
      int i_A;
      int i_B;
      int i_C;
      int i_V;
      int i_W;

  };

}
