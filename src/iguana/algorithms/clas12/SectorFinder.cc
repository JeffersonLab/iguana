#include "SectorFinder.h"

namespace iguana::clas12 {

  REGISTER_IGUANA_ALGORITHM(SectorFinder, "REC::Particle::Sector");

  void SectorFinder::Start(hipo::banklist& banks)
  {

    // define options, their default values, and cache them
    ParseYAMLConfig();
    o_bankname = GetOptionScalar<std::string>("bank");

    // get expected bank indices
    b_particle = GetBankIndex(banks, "REC::Particle");
    if(o_bankname != "default") {
      b_user            = GetBankIndex(banks, o_bankname);
      userSpecifiedBank = true;
    }
    else {
      b_calorimeter     = GetBankIndex(banks, "REC::Calorimeter");
      b_track           = GetBankIndex(banks, "REC::Track");
      b_scint           = GetBankIndex(banks, "REC::Scintillator");
      userSpecifiedBank = false;
    }

    // create the output bank
    // FIXME: generalize the groupid and itemid
    auto result_schema = CreateBank(banks, b_result, "REC::Particle::Sector", {"sector/I"}, 0xF000, 2);
    i_sector = result_schema.getEntryOrder("sector");
  }


  void SectorFinder::Run(hipo::banklist& banks) const
  {
    auto& particleBank = GetBank(banks, b_particle, "REC::Particle");
    auto& resultBank   = GetBank(banks, b_result, "REC::Particle::Sector");

    // sync new bank with particle bank, and fill it with zeroes
    resultBank.setRows(particleBank.getRows());
    resultBank.getMutableRowList().setList(particleBank.getRowList());
    for(int row = 0; row < resultBank.getRows(); row++)
      resultBank.putInt(i_sector, row, 0);

    // filter the input bank for requested PDG code(s)
    if(userSpecifiedBank) { // if user specified a specific bank
      auto const& userBank = GetBank(banks, b_user);
      for(auto const& row : particleBank.getRowList()) {
        if(userBank.getRowList().size() > 0) {
          resultBank.putInt(i_sector, row, GetSector(userBank, row));
        }
      }
    }
    else { // use the standard method
      auto const& calBank   = GetBank(banks, b_calorimeter);
      auto const& scintBank = GetBank(banks, b_scint);
      auto const& trackBank = GetBank(banks, b_track);
      for(auto const& row : particleBank.getRowList()) {
        int trackSector = 0;
        if(trackBank.getRowList().size() > 0) {
          trackSector = GetSector(trackBank, row);
        }

        int scintSector = 0;
        if(scintBank.getRowList().size() > 0) {
          scintSector = GetSector(scintBank, row);
        }

        int calSector = 0;
        if(calBank.getRowList().size() > 0) {
          calSector = GetSector(calBank, row);
        }

        if(trackSector != 0) {
          resultBank.putInt(i_sector, row, trackSector);
        }
        else if(scintSector != 0) {
          resultBank.putInt(i_sector, row, scintSector);
        }
        else {
          // FIXME: add even if calSector is 0
          // need an entry per pindex
          // can happen that particle not in FD
          // ie sector is 0
          resultBank.putInt(i_sector, row, calSector);
        }
      }
    }

    ShowBank(resultBank, Logger::Header("CREATED BANK"));
  }

  int SectorFinder::GetSector(hipo::bank const& bank, int const pindex) const
  {
    int sector = 0;
    for(auto const& row : bank.getRowList()) {
      if(bank.getInt("pindex", row) == pindex) {
        return bank.getInt("sector", row);
      }
    }
    return sector;
  }

  void SectorFinder::Stop()
  {
  }

}
