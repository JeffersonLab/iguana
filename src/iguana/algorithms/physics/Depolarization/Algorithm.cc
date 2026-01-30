#include "Algorithm.h"

namespace iguana::physics {

  REGISTER_IGUANA_ALGORITHM(Depolarization, "physics::Depolarization");

  ///////////////////////////////////////////////////////////////////////////////

  void Depolarization::StartHook(hipo::banklist& banks)
  {
    b_inc_kin = GetBankIndex(banks, "physics::InclusiveKinematics");

    // create the output bank
    auto result_schema = CreateBank(banks, b_result, GetClassName());
    i_epsilon          = result_schema.getEntryOrder("epsilon");
    i_A                = result_schema.getEntryOrder("A");
    i_B                = result_schema.getEntryOrder("B");
    i_C                = result_schema.getEntryOrder("C");
    i_V                = result_schema.getEntryOrder("V");
    i_W                = result_schema.getEntryOrder("W");
  }

  ///////////////////////////////////////////////////////////////////////////////

  bool Depolarization::RunHook(hipo::banklist& banks) const
  {
    return Run(
        GetBank(banks, b_inc_kin, "physics::InclusiveKinematics"),
        GetBank(banks, b_result, GetClassName()));
  }

  bool Depolarization::Run(
      hipo::bank const& inc_kin_bank,
      hipo::bank& result_bank) const
  {
    result_bank.reset(); // IMPORTANT: always first `reset` the created bank(s)
    ShowBank(inc_kin_bank, Logger::Header("INPUT INCLUSIVE KINEMATICS"));

    // set `result_bank` rows and rowlist to match those of `inc_kin_bank`
    auto const& inc_kin_bank_rowlist = inc_kin_bank.getRowList();
    result_bank.setRows(inc_kin_bank.getRows());
    result_bank.getMutableRowList().setList(inc_kin_bank_rowlist);

    // loop over ALL `inc_kin_bank`'s rows; calculate depolarization for only the rows
    // that are in its current rowlist, and zero the rest
    for(int row = 0; row < inc_kin_bank.getRows(); row++) {
      if(std::find(inc_kin_bank_rowlist.begin(), inc_kin_bank_rowlist.end(), row) != inc_kin_bank_rowlist.end()) {
        auto result_vars = Compute(
            inc_kin_bank.getDouble("Q2", row),
            inc_kin_bank.getDouble("x", row),
            inc_kin_bank.getDouble("y", row),
            inc_kin_bank.getDouble("targetM", row));
        result_bank.putDouble(i_epsilon, row, result_vars.epsilon);
        result_bank.putDouble(i_A, row, result_vars.A);
        result_bank.putDouble(i_B, row, result_vars.B);
        result_bank.putDouble(i_C, row, result_vars.C);
        result_bank.putDouble(i_V, row, result_vars.V);
        result_bank.putDouble(i_W, row, result_vars.W);
      }
      else {
        result_bank.putDouble(i_epsilon, row, 0);
        result_bank.putDouble(i_A, row, 0);
        result_bank.putDouble(i_B, row, 0);
        result_bank.putDouble(i_C, row, 0);
        result_bank.putDouble(i_V, row, 0);
        result_bank.putDouble(i_W, row, 0);
      }
    }

    ShowBank(result_bank, Logger::Header("CREATED BANK"));
    return true;
  }

  ///////////////////////////////////////////////////////////////////////////////

  DepolarizationVars Depolarization::Compute(double const Q2, double const x, double const y, double const targetM) const
  {
    DepolarizationVars const zero_result{
        .epsilon = 0,
        .A       = 0,
        .B       = 0,
        .C       = 0,
        .V       = 0,
        .W       = 0};

    // calculate gamma
    if(Q2 <= 0) {
      m_log->Warn("Q2 = {} <= 0", Q2);
      return zero_result;
    }
    auto gamma = 2 * targetM * x / std::sqrt(Q2);

    // calculate epsilon
    auto epsilon_denom = 1 - y + y * y / 2 + std::pow(gamma * y, 2) / 4;
    if(!(std::abs(epsilon_denom) > 0)) {
      m_log->Warn("epsilon denominator is zero");
      return zero_result;
    }
    auto epsilon = (1 - y - std::pow(gamma * y, 2) / 4) / epsilon_denom;

    // calculate A
    auto A_denom = 2 - 2 * epsilon;
    if(!(std::abs(A_denom) > 0)) {
      m_log->Warn("depol. factor A denominator is zero");
      return zero_result;
    }
    auto A = y * y / A_denom;

    // calculate B,C,V,W
    return {
        .epsilon = epsilon,
        .A       = A,
        .B       = A * epsilon,
        .C       = A * std::sqrt(1 - epsilon * epsilon),
        .V       = A * std::sqrt(2 * epsilon * (1 + epsilon)),
        .W       = A * std::sqrt(2 * epsilon * (1 - epsilon))};
  }

  ///////////////////////////////////////////////////////////////////////////////

}
