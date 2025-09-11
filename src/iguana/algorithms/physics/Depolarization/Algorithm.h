#pragma once

#include "iguana/algorithms/Algorithm.h"

namespace iguana::physics {

  /// @brief_algo Calculate depolarization factors
  ///
  /// @par References
  /// - https://arxiv.org/pdf/hep-ph/0611265
  /// - https://arxiv.org/pdf/1408.5721
  ///
  /// @begin_doc_algo{physics::Depolarization | Creator}
  /// @input_banks{%physics::InclusiveKinematics}
  /// @output_banks{%physics::Depolarization}
  /// @end_doc
  ///
  /// @creator_note
  class Depolarization : public Algorithm
  {

      DEFINE_IGUANA_ALGORITHM(Depolarization, physics::Depolarization)

    public:

      void Start(hipo::banklist& banks) override;
      bool Run(hipo::banklist& banks) const override;
      void Stop() override;

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
