#include "Algorithm.h"
#include "iguana/algorithms/physics/Tools.h"

namespace iguana::clas12 {

  REGISTER_IGUANA_ALGORITHM(MatchParticleProximity, "clas12::MatchParticleProximity");

  ///////////////////////////////////////////////////////////////////////////////

  void MatchParticleProximity::ConfigHook()
  {
    o_bank_a = GetOptionScalar<std::string>({"bank_a"});
    o_bank_b = GetOptionScalar<std::string>({"bank_b"});
  }

  ///////////////////////////////////////////////////////////////////////////////

  void MatchParticleProximity::StartHook(hipo::banklist& banks)
  {
    // banklist indices
    b_bank_a = GetBankIndex(banks, o_bank_a);
    b_bank_b = GetBankIndex(banks, o_bank_b);

    // create the output bank
    auto result_schema = CreateBank(banks, b_result, "clas12::MatchParticleProximity");
    i_pindex_a         = result_schema.getEntryOrder("pindex_a");
    i_pindex_b         = result_schema.getEntryOrder("pindex_b");
    i_proximity        = result_schema.getEntryOrder("proximity");
  }

  ///////////////////////////////////////////////////////////////////////////////

  bool MatchParticleProximity::RunHook(hipo::banklist& banks) const
  {
    return Run(
        GetBank(banks, b_bank_a, o_bank_a),
        GetBank(banks, b_bank_b, o_bank_b),
        GetBank(banks, b_result, "clas12::MatchParticleProximity"));
  }

  bool MatchParticleProximity::Run(
      hipo::bank const& bank_a,
      hipo::bank const& bank_b,
      hipo::bank& result_bank) const
  {
    result_bank.reset(); // IMPORTANT: always first `reset` the created bank(s)

    ShowBank(bank_a, Logger::Header("INPUT BANK A"));
    ShowBank(bank_b, Logger::Header("INPUT BANK B"));

    // output rows
    std::vector<MatchParticleProximityVars> result_rows;

    // loop over ALL bank-A particles, to find matching bank-B particles
    for(int row_a = 0; row_a < bank_a.getRows(); row_a++) {

      // matching variables
      double min_prox = -1; // minimum proximity
      int pindex_b    = -1; // matching `pindex` of `bank_b`

      // particle info
      auto pid_a = bank_a.getInt("pid", row_a);
      ROOT::Math::XYZVector p_a(
          bank_a.getFloat("px", row_a),
          bank_a.getFloat("py", row_a),
          bank_a.getFloat("pz", row_a));
      auto theta_a = p_a.theta();
      auto phi_a   = p_a.phi();

      // loop over ALL bank-B particles, and find the one with the smallest proximity to
      // the current bank-A particle
      for(int row_b = 0; row_b < bank_b.getRows(); row_b++) {
        // PID must match
        if(pid_a == bank_b.getInt("pid", row_b)) {
          // particle info
          ROOT::Math::XYZVector p_b(
              bank_b.getFloat("px", row_b),
              bank_b.getFloat("py", row_b),
              bank_b.getFloat("pz", row_b));
          auto theta_b = p_b.theta();
          auto phi_b   = p_b.phi();
          // calculate Euclidean distance in (theta,phi) space
          auto prox = std::hypot(
              physics::tools::AdjustAnglePi(theta_b - theta_a),
              physics::tools::AdjustAnglePi(phi_b - phi_a));
          // if smallest proximity, this is the best one
          if(min_prox < 0 || prox < min_prox) {
            min_prox = prox;
            pindex_b = row_b;
          }
        }
      }

      // if a match was found, populate the output bank
      if(pindex_b >= 0)
        result_rows.push_back({
            .pindex_a  = row_a,
            .pindex_b  = pindex_b,
            .proximity = min_prox,
        });
    }

    // fill output bank
    result_bank.setRows(result_rows.size());
    for(decltype(result_rows)::size_type row = 0; row < result_rows.size(); row++) {
      auto const& result_row = result_rows.at(row);
      result_bank.putShort(i_pindex_a, row, result_row.pindex_a);
      result_bank.putShort(i_pindex_b, row, result_row.pindex_b);
      result_bank.putDouble(i_proximity, row, result_row.proximity);
    }

    ShowBank(result_bank, Logger::Header("CREATED BANK"));
    return result_bank.getRows() > 0;
  }

  ///////////////////////////////////////////////////////////////////////////////

}
