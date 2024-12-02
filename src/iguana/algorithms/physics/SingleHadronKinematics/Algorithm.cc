#include "Algorithm.h"
#include "TypeDefs.h"
#include "iguana/algorithms/physics/Tools.h"

#include <Math/Boost.h>

namespace iguana::physics {

  REGISTER_IGUANA_ALGORITHM(SingleHadronKinematics, "physics::SingleHadronKinematics");

  void SingleHadronKinematics::Start(hipo::banklist& banks)
  {
    b_particle = GetBankIndex(banks, "REC::Particle");
    b_inc_kin  = GetBankIndex(banks, "physics::InclusiveKinematics");

    // create the output bank
    // FIXME: generalize the groupid and itemid
    auto result_schema = CreateBank(
        banks,
        b_result,
        GetClassName(),
        {
          "pindex/S",
          "pdg/I",
          "z/D",
          "PhPerp/D",
          "MX2/D",
          "xF/D",
          "yB/D",
          "phiH/D",
          "xi/D"
        },
        0xF000,
        7);
    i_pindex = result_schema.getEntryOrder("pindex");
    i_pdg    = result_schema.getEntryOrder("pdg");
    i_z      = result_schema.getEntryOrder("z");
    i_PhPerp = result_schema.getEntryOrder("PhPerp");
    i_MX2    = result_schema.getEntryOrder("MX2");
    i_xF     = result_schema.getEntryOrder("xF");
    i_yB     = result_schema.getEntryOrder("yB");
    i_phiH   = result_schema.getEntryOrder("phiH");
    i_xi     = result_schema.getEntryOrder("xi");

    // parse config file
    ParseYAMLConfig();
    o_hadron_pdgs = GetOptionSet<int>("hadron_list");

  }

  ///////////////////////////////////////////////////////////////////////////////

  void SingleHadronKinematics::Run(hipo::banklist& banks) const
  {
    auto& particle_bank = GetBank(banks, b_particle, "REC::Particle");
    auto& inc_kin_bank  = GetBank(banks, b_inc_kin, "physics::InclusiveKinematics");
    auto& result_bank   = GetBank(banks, b_result, GetClassName());
    ShowBank(particle_bank, Logger::Header("INPUT PARTICLES"));

    if(particle_bank.getRowList().empty() || inc_kin_bank.getRowList().empty()) {
      m_log->Debug("skip this event, since not all required banks have entries");
      return;
    }

    // get beam and target momenta
    // FIXME: makes some assumptions about the beam; this should be generalized...
    ROOT::Math::PxPyPzMVector p_beam(
        0.0,
        0.0,
        inc_kin_bank.getDouble("beamPz", 0),
        particle::mass.at(particle::electron));
    ROOT::Math::PxPyPzMVector p_target(
        0.0,
        0.0,
        0.0,
        inc_kin_bank.getDouble("targetM", 0));

    // get virtual photon momentum
    ROOT::Math::PxPyPzEVector p_q(
        inc_kin_bank.getDouble("qx", 0),
        inc_kin_bank.getDouble("qy", 0),
        inc_kin_bank.getDouble("qz", 0),
        inc_kin_bank.getDouble("qE", 0));

    // get additional inclusive variables
    auto x = inc_kin_bank.getDouble("x", 0);
    auto W = inc_kin_bank.getDouble("W", 0);

    // boosts
    ROOT::Math::Boost boost__qp((p_q + p_target).BoostToCM()); // CoM frame of target and virtual photon
    ROOT::Math::Boost boost__breit((p_q + 2 * x * p_target).BoostToCM()); // Breit frame
    auto p_q__qp    = boost__qp(p_q);
    auto p_q__breit = boost__breit(p_q);

    // banks' row lists
    auto const& particle_bank_rowlist = particle_bank.getRowList();
    hipo::bank::rowlist::list_t result_bank_rowlist{};
    result_bank.setRows(particle_bank.getRows());

    // loop over ALL rows of `particle_bank`
    // - we will calculate kinematics for rows in `particle_bank_rowlist`, and zero out all the other rows
    // - we want the `result_bank` to have the same number of rows as `particle_bank` and the same ordering,
    //   so that banks which reference `particle_bank` rows can be used to reference `result_bank` rows too
    for(int row = 0; row < particle_bank.getRows(); row++) {

      // if the particle is in `o_hadron_pdgs` AND the row is in `particle_bank`'s filtered row list
      if(auto pdg{particle_bank.getInt("pid", row)};
          o_hadron_pdgs.find(pdg) != o_hadron_pdgs.end() &&
          std::find(particle_bank_rowlist.begin(), particle_bank_rowlist.end(), row) != particle_bank_rowlist.end()) {

        // hadron momentum
        auto p_Ph = ROOT::Math::PxPyPzMVector(
            particle_bank.getFloat("px", row),
            particle_bank.getFloat("py", row),
            particle_bank.getFloat("pz", row),
            particle::mass.at(static_cast<particle::PDG>(pdg)));
        auto p_Ph__qp    = boost__qp(p_Ph);
        auto p_Ph__breit = boost__breit(p_Ph);

        // calculate z
        double z = p_target.Dot(p_Ph) / p_target.Dot(p_q);

        // calculate PhPerp
        auto opt_PhPerp = tools::RejectVector(p_Ph.Vect(), p_q.Vect());
        double PhPerp   = opt_PhPerp.has_value() ? opt_PhPerp.value().R() : tools::UNDEF;

        // calculate MX2
        double MX2 = (p_target + p_q - p_Ph).M2();

        // calculate xF
        double xF = 2 * p_Ph__qp.Vect().Dot(p_q__qp.Vect()) / (W * p_q__qp.Vect().R());

        // calculate yB
        double yB = tools::ParticleRapidity(p_Ph__breit, p_q__breit.Vect()).value_or(tools::UNDEF);

        // calculate phiH
        double phiH = tools::PlaneAngle(
            p_q.Vect(),
            p_beam.Vect(),
            p_q.Vect(),
            p_Ph.Vect()).value_or(tools::UNDEF);

        // calculate xi
        double xi = p_q.Dot(p_Ph) / p_target.Dot(p_q);

        // put this particle in `result_bank`'s row list
        result_bank_rowlist.push_back(row);

        // fill the bank
        result_bank.putShort(i_pindex,  row, static_cast<int16_t>(row));
        result_bank.putInt(i_pdg,       row, pdg);
        result_bank.putDouble(i_z,      row, z);
        result_bank.putDouble(i_PhPerp, row, PhPerp);
        result_bank.putDouble(i_MX2,    row, MX2);
        result_bank.putDouble(i_xF,     row, xF);
        result_bank.putDouble(i_yB,     row, yB);
        result_bank.putDouble(i_phiH,   row, phiH);
        result_bank.putDouble(i_xi,     row, xi);
      }
      else {
        // zero the row
        result_bank.putShort(i_pindex,  row, static_cast<int16_t>(row));
        result_bank.putInt(i_pdg,       row, pdg);
        result_bank.putDouble(i_z,      row, 0);
        result_bank.putDouble(i_PhPerp, row, 0);
        result_bank.putDouble(i_MX2,    row, 0);
        result_bank.putDouble(i_xF,     row, 0);
        result_bank.putDouble(i_yB,     row, 0);
        result_bank.putDouble(i_phiH,   row, 0);
        result_bank.putDouble(i_xi,     row, 0);
      }
    }

    // apply the filtered rowlist to `result_bank`
    result_bank.getMutableRowList().setList(result_bank_rowlist);

    ShowBank(result_bank, Logger::Header("CREATED BANK"));
  }

  ///////////////////////////////////////////////////////////////////////////////

  void SingleHadronKinematics::Stop()
  {
  }

}
