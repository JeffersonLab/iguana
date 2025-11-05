#include "Algorithm.h"
#include "iguana/algorithms/physics/Tools.h"

namespace iguana::clas12 {

  REGISTER_IGUANA_ALGORITHM(MCProximityMatch, "MC::RecMatch::Proximity");

  void MCProximityMatch::Start(hipo::banklist& banks)
  {
    b_rec_particle_bank = GetBankIndex(banks, "REC::Particle");
    b_mc_particle_bank  = GetBankIndex(banks, "MC::Particle");

    // create the output bank
    auto result_schema = CreateBank(banks, b_result, "MC::RecMatch::Proximity");
    i_pindex           = result_schema.getEntryOrder("pindex");
    i_mcindex          = result_schema.getEntryOrder("mcindex");
    i_proximity        = result_schema.getEntryOrder("proximity");
  }

  ///////////////////////////////////////////////////////////////////////////////

  bool MCProximityMatch::Run(hipo::banklist& banks) const
  {
    return Run(
        GetBank(banks, b_rec_particle_bank, "REC::Particle"),
        GetBank(banks, b_mc_particle_bank, "MC::Particle"),
        GetBank(banks, b_result, "MC::RecMatch::Proximity"));
  }

  bool MCProximityMatch::Run(
      hipo::bank const& rec_particle_bank,
      hipo::bank const& mc_particle_bank,
      hipo::bank& result_bank) const
  {
    ShowBank(rec_particle_bank, Logger::Header("INPUT RECONSTRUCTED PARTICLES"));
    ShowBank(mc_particle_bank, Logger::Header("INPUT GENERATED PARTICLE"));

    // output rows
    std::vector<MCProximityMatchVars> result_rows;

    // loop over ALL reconstructed particles, to find matching generated particles
    for(int row_rec = 0; row_rec < rec_particle_bank.getRows(); row_rec++) {

      // matching variables
      double min_prox = -1; // minimum proximity
      int pindex_mc   = -1; // matching `pindex` of `mc_particle_bank`

      // reconstructed particle info
      auto pid_rec = rec_particle_bank.getInt("pid", row_rec);
      ROOT::Math::XYZVector p_rec(
          rec_particle_bank.getFloat("px", row_rec),
          rec_particle_bank.getFloat("py", row_rec),
          rec_particle_bank.getFloat("pz", row_rec));
      auto theta_rec = p_rec.theta();
      auto phi_rec   = p_rec.phi();

      // loop over ALL generated particles, and find the one with the smallest proximity to
      // the current reconstructed particle
      for(int row_mc = 0; row_mc < mc_particle_bank.getRows(); row_mc++) {

        // PID must match
        if(pid_rec == mc_particle_bank.getInt("pid", row_mc)) {
          // generated particle info
          ROOT::Math::XYZVector p_mc(
              mc_particle_bank.getFloat("px", row_mc),
              mc_particle_bank.getFloat("py", row_mc),
              mc_particle_bank.getFloat("pz", row_mc));
          auto theta_mc = p_mc.theta();
          auto phi_mc   = p_mc.phi();
          // calculate Euclidean distance in (theta,phi) space
          auto prox = std::hypot(
              physics::tools::AdjustAnglePi(theta_mc - theta_rec),
              physics::tools::AdjustAnglePi(phi_mc - phi_rec));
          // if smallest proximity, this is the best one
          if(min_prox < 0 || prox < min_prox) {
            min_prox  = prox;
            pindex_mc = row_mc;
          }
        }
      }

      // if a match was found, populate the output bank
      if(pindex_mc >= 0)
        result_rows.push_back({
            .pindex    = row_rec,
            .mcindex   = pindex_mc,
            .proximity = min_prox,
        });
    }

    // fill output bank
    result_bank.setRows(result_rows.size());
    for(decltype(result_rows)::size_type row = 0; row < result_rows.size(); row++) {
      auto const& result_row = result_rows.at(row);
      result_bank.putShort(i_pindex, row, result_row.pindex);
      result_bank.putShort(i_mcindex, row, result_row.mcindex);
      result_bank.putDouble(i_proximity, row, result_row.proximity);
    }

    ShowBank(result_bank, Logger::Header("CREATED BANK"));
    return result_bank.getRows() > 0;
  }

  ///////////////////////////////////////////////////////////////////////////////

  void MCProximityMatch::Stop()
  {
  }

}
