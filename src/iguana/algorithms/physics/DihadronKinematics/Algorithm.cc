#include "Algorithm.h"
#include "TypeDefs.h"

#include <Math/Boost.h>

namespace iguana::physics {

  REGISTER_IGUANA_ALGORITHM(DihadronKinematics, "physics::DihadronKinematics");

  void DihadronKinematics::Start(hipo::banklist& banks)
  {
    b_particle = GetBankIndex(banks, "REC::Particle");
    b_inc_kin  = GetBankIndex(banks, "physics::InclusiveKinematics");

    // create the output bank
    auto result_schema = CreateBank(banks, b_result, GetClassName());
    i_pindex_a = result_schema.getEntryOrder("pindex_a");
    i_pindex_b = result_schema.getEntryOrder("pindex_b");
    i_pdg_a    = result_schema.getEntryOrder("pdg_a");
    i_pdg_b    = result_schema.getEntryOrder("pdg_b");
    i_Mh       = result_schema.getEntryOrder("Mh");
    i_z        = result_schema.getEntryOrder("z");
    i_PhPerp   = result_schema.getEntryOrder("PhPerp");
    i_MX2      = result_schema.getEntryOrder("MX2");
    i_xF       = result_schema.getEntryOrder("xF");
    i_yB       = result_schema.getEntryOrder("yB");
    i_phiH     = result_schema.getEntryOrder("phiH");
    i_phiR     = result_schema.getEntryOrder("phiR");
    i_theta    = result_schema.getEntryOrder("theta");

    // parse config file
    ParseYAMLConfig();
    o_hadron_a_pdgs = GetOptionSet<int>("hadron_a_list");
    o_hadron_b_pdgs = GetOptionSet<int>("hadron_b_list");
    o_phi_r_method  = GetOptionScalar<std::string>("phi_r_method");
    o_theta_method  = GetOptionScalar<std::string>("theta_method");

    // check phiR method
    if(o_phi_r_method == "RT_via_covariant_kT")
      m_phi_r_method = e_RT_via_covariant_kT;
    else
      throw std::runtime_error(fmt::format("unknown phi_r_method: {:?}", o_phi_r_method));

    // check theta method
    if(o_theta_method == "hadron_a")
      m_theta_method = e_hadron_a;
    else
      throw std::runtime_error(fmt::format("unknown theta_method: {:?}", o_theta_method));

  }

  ///////////////////////////////////////////////////////////////////////////////

  void DihadronKinematics::Run(hipo::banklist& banks) const
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

    // build list of dihadron rows (pindices)
    auto dih_rows = PairHadrons(particle_bank);

    // loop over dihadrons
    result_bank.setRows(dih_rows.size());
    int dih_row = 0;
    for(const auto& [row_a, row_b] : dih_rows) {

      // get hadron momenta
      Hadron had_a{.row = row_a};
      Hadron had_b{.row = row_b};
      for(auto& had : {&had_a, &had_b}) {
        had->pdg = particle_bank.getInt("pid", had->row);
        had->p   = ROOT::Math::PxPyPzMVector(
            particle_bank.getFloat("px", had->row),
            particle_bank.getFloat("py", had->row),
            particle_bank.getFloat("pz", had->row),
            particle::mass.at(static_cast<particle::PDG>(had->pdg)));
      }

      // calculate dihadron momenta and boosts
      auto p_Ph        = had_a.p + had_b.p;
      auto p_Ph__qp    = boost__qp(p_Ph);
      auto p_Ph__breit = boost__breit(p_Ph);
      ROOT::Math::Boost boost__dih(p_Ph.BoostToCM()); // CoM frame of dihadron

      // calculate z
      double z = p_target.Dot(p_Ph) / p_target.Dot(p_q);

      // calculate PhPerp
      auto opt_PhPerp = tools::RejectVector(p_Ph.Vect(), p_q.Vect());
      double PhPerp   = opt_PhPerp.has_value() ? opt_PhPerp.value().R() : tools::UNDEF;

      // calculate Mh
      double Mh = p_Ph.M();

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

      // calculate PhiR
      double phiR = tools::UNDEF;
      switch(m_phi_r_method) {
      case e_RT_via_covariant_kT:
        {
          for(auto& had : {&had_a, &had_b}) {
            had->z      = p_target.Dot(had->p) / p_target.Dot(p_q);
            had->p_perp = tools::RejectVector(had->p.Vect(), p_q.Vect());
          }
          if(had_a.p_perp.has_value() && had_b.p_perp.has_value()) {
            auto RT = (had_b.z * had_a.p_perp.value() - had_a.z * had_b.p_perp.value()) / (had_a.z + had_b.z);
            phiR = tools::PlaneAngle(
                p_q.Vect(),
                p_beam.Vect(),
                p_q.Vect(),
                RT).value_or(tools::UNDEF);
          }
          break;
        }
      }

      // calculate theta
      double theta = tools::UNDEF;
      switch(m_theta_method) {
      case e_hadron_a:
        {
          theta = tools::VectorAngle(
              boost__dih(had_a.p).Vect(),
              p_Ph.Vect()).value_or(tools::UNDEF);
          break;
        }
      }

      result_bank.putShort(i_pindex_a, dih_row, static_cast<int16_t>(had_a.row));
      result_bank.putShort(i_pindex_b, dih_row, static_cast<int16_t>(had_b.row));
      result_bank.putInt(i_pdg_a,      dih_row, had_a.pdg);
      result_bank.putInt(i_pdg_b,      dih_row, had_b.pdg);
      result_bank.putDouble(i_Mh,      dih_row, Mh);
      result_bank.putDouble(i_z,       dih_row, z);
      result_bank.putDouble(i_PhPerp,  dih_row, PhPerp);
      result_bank.putDouble(i_MX2,     dih_row, MX2);
      result_bank.putDouble(i_xF,      dih_row, xF);
      result_bank.putDouble(i_yB,      dih_row, yB);
      result_bank.putDouble(i_phiH,    dih_row, phiH);
      result_bank.putDouble(i_phiR,    dih_row, phiR);
      result_bank.putDouble(i_theta,   dih_row, theta);

      dih_row++;
    }

    ShowBank(result_bank, Logger::Header("CREATED BANK"));
  }

  ///////////////////////////////////////////////////////////////////////////////

  std::vector<std::pair<int,int>> DihadronKinematics::PairHadrons(hipo::bank const& particle_bank) const {
    std::vector<std::pair<int,int>> result;
    // loop over REC::Particle rows, for hadron A
    for(auto const& row_a : particle_bank.getRowList()) {
      // check PDG is in the hadron-A list
      if(auto pdg_a{particle_bank.getInt("pid", row_a)}; o_hadron_a_pdgs.find(pdg_a) != o_hadron_a_pdgs.end()) {
        // loop over REC::Particle rows, for hadron B
        for(auto const& row_b : particle_bank.getRowList()) {
          // don't pair a particle with itself
          if(row_a == row_b)
            continue;
          // check PDG is in the hadron-B list
          if(auto pdg_b{particle_bank.getInt("pid", row_b)}; o_hadron_b_pdgs.find(pdg_b) != o_hadron_b_pdgs.end()) {
            // if the PDGs of hadrons A and B are the same, don't double count
            if(pdg_a == pdg_b && row_b < row_a)
              continue;
            // we have a unique dihadron, add it to the list
            result.push_back({row_a, row_b});
          }
        }
      }
    }
    // trace logging
    if(m_log->GetLevel() <= Logger::Level::trace) {
      if(result.empty())
        m_log->Trace("=> no dihadrons in this event");
      else
        m_log->Trace("=> number of dihadrons found: {}", result.size());
    }
    return result;
  }

  ///////////////////////////////////////////////////////////////////////////////

  void DihadronKinematics::Stop()
  {
  }

}
