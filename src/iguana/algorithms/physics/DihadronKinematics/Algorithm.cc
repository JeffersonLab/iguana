#include "Algorithm.h"

// ROOT
#include <Math/Vector4D.h>

namespace iguana::physics {

  REGISTER_IGUANA_ALGORITHM(DihadronKinematics, "physics::DihadronKinematics");

  void DihadronKinematics::Start(hipo::banklist& banks)
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
          "pindex_a/S",
          "pindex_b/S",
          "pdg_a/I",
          "pdg_b/I",
          "Mh/D",
          "z/D",
          "MX/D",
          "xF/D",
          "phiH/D",
          "phiR/D"
        },
        0xF000,
        5);
    i_pindex_a = result_schema.getEntryOrder("pindex_a");
    i_pindex_b = result_schema.getEntryOrder("pindex_b");
    i_pdg_a    = result_schema.getEntryOrder("pdg_a");
    i_pdg_b    = result_schema.getEntryOrder("pdg_b");
    i_Mh       = result_schema.getEntryOrder("Mh");
    i_z        = result_schema.getEntryOrder("z");
    i_MX       = result_schema.getEntryOrder("MX");
    i_xF       = result_schema.getEntryOrder("xF");
    i_phiH     = result_schema.getEntryOrder("phiH");
    i_phiR     = result_schema.getEntryOrder("phiR");

    // parse config file
    ParseYAMLConfig();
    o_hadron_a_pdgs = GetOptionSet<int>("hadron_a_list");
    o_hadron_b_pdgs = GetOptionSet<int>("hadron_b_list");
    o_phi_r_method  = GetOptionScalar<std::string>("phi_r_method");

  }

  ///////////////////////////////////////////////////////////////////////////////

  void DihadronKinematics::Run(hipo::banklist& banks) const
  {
    auto& particle_bank = GetBank(banks, b_particle, "REC::Particle");
    auto& inc_kin_bank  = GetBank(banks, b_inc_kin, "physics::InclusiveKinematics");
    auto& result_bank   = GetBank(banks, b_result, GetClassName());
    ShowBank(particle_bank, Logger::Header("INPUT PARTICLES"));


    // build list of dihadron rows (pindices)
    std::vector<std::pair<int,int>> dih_rows;
    for(auto const& row_a : particle_bank.getRowList()) {
      // check PDG is in the hadron-A list
      if(o_hadron_a_pdgs.find(particle_bank.getInt("pid", row_a)) == o_hadron_a_pdgs.end())
        continue;
      for(auto const& row_b : particle_bank.getRowList()) {
        // don't pair a particle with itself
        if(row_a == row_b)
          continue;
        // check PDG is in the hadron-B list
        if(o_hadron_b_pdgs.find(particle_bank.getInt("pid", row_b)) == o_hadron_b_pdgs.end())
          continue;
        dih_rows.push_back({row_a, row_b});
      }
    }

    result_bank.setRows(dih_rows.size());

    int dih_row = 0;
    for(const auto& [row_a, row_b] : dih_rows) {

      result_bank.putShort(i_pindex_a, dih_row, row_a);
      result_bank.putShort(i_pindex_b, dih_row, row_b);
      // result_bank.putInt(i_pdg_a,      dih_row, 0);
      // result_bank.putInt(i_pdg_b,      dih_row, 0);
      // result_bank.putDouble(i_Mh,      dih_row, 0);
      // result_bank.putDouble(i_z,       dih_row, 0);
      // result_bank.putDouble(i_MX,      dih_row, 0);
      // result_bank.putDouble(i_xF,      dih_row, 0);
      // result_bank.putDouble(i_phiH,    dih_row, 0);
      // result_bank.putDouble(i_phiR,    dih_row, 0);

      dih_row++;
    }

    ShowBank(result_bank, Logger::Header("CREATED BANK"));
  }

  ///////////////////////////////////////////////////////////////////////////////

  void DihadronKinematics::Stop()
  {
  }

}
