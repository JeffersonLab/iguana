#include "Algorithm.h"

namespace iguana::clas12 {

  REGISTER_IGUANA_ALGORITHM(SectorFinder, "REC::Particle::Sector");

  void SectorFinder::Start(hipo::banklist& banks)
  {

    // define options, their default values, and cache them
    ParseYAMLConfig();
    o_bankname_charged = GetOptionScalar<std::string>("bank_charged");
    o_bankname_uncharged = GetOptionScalar<std::string>("bank_uncharged");

    bool setDefaultBanks=false;
    // get expected bank indices
    b_particle = GetBankIndex(banks, "REC::Particle");
    if(o_bankname_charged != "default") {
      b_user_charged            = GetBankIndex(banks, o_bankname_charged);
      userSpecifiedBank_charged = true;
    }
    else {
      b_calorimeter     = GetBankIndex(banks, "REC::Calorimeter");
      b_track           = GetBankIndex(banks, "REC::Track");
      b_scint           = GetBankIndex(banks, "REC::Scintillator");
      setDefaultBanks   = true;
      userSpecifiedBank_charged = false;
    }

    if(o_bankname_uncharged != "default") {
      b_user_uncharged            = GetBankIndex(banks, o_bankname_uncharged);
      userSpecifiedBank_uncharged = true;
    }
    else {
      //avoid setting default banks twice
      if(!setDefaultBanks){
        b_calorimeter     = GetBankIndex(banks, "REC::Calorimeter");
        b_track           = GetBankIndex(banks, "REC::Track");
        b_scint           = GetBankIndex(banks, "REC::Scintillator");
        setDefaultBanks = true;
      }
      userSpecifiedBank_uncharged = false;
    }

    // create the output bank
    // FIXME: generalize the groupid and itemid
    auto result_schema = CreateBank(banks, b_result, "REC::Particle::Sector", {"sector/I"}, 0xF000, 2);
    i_sector = result_schema.getEntryOrder("sector");
  }


  void SectorFinder::setSector(hipo::banklist& banks,hipo::bank& resultBank,int row, bool userSpecifiedBank, hipo::banklist::size_type b_user) const
  {
    // filter the input bank for requested PDG code(s)
    if(userSpecifiedBank) { // if user specified a specific bank
      auto const& userBank = GetBank(banks, b_user);
      int sect=GetSector(userBank, row);
      //NB: Sector added only if found
      //Otherwise output bank already has 0 at that row
      if (sect!=-1)
        resultBank.putInt(i_sector, row, sect);
    }
    else { // use the standard method
      auto const& calBank   = GetBank(banks, b_calorimeter);
      auto const& scintBank = GetBank(banks, b_scint);
      auto const& trackBank = GetBank(banks, b_track);
      
      int trackSector = GetSector(trackBank, row);
      int scintSector = GetSector(scintBank, row);
      int calSector = GetSector(calBank, row);

      //NB: Sector added only if found
      //Otherwise output bank already has 0 at that row
      if(trackSector != -1) {
        resultBank.putInt(i_sector, row, trackSector);
      }
      else if(scintSector != -1) {
        resultBank.putInt(i_sector, row, scintSector);
      }
      else if(calSector != -1){
        resultBank.putInt(i_sector, row, calSector);
      }
    }
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


    for(int row = 0; row < particleBank.getRows(); row++) {
      int charge=particleBank.getInt("charge",row);
      if(charge==0){
        setSector(banks,resultBank,row,userSpecifiedBank_uncharged,b_user_uncharged);
      } else {
        setSector(banks,resultBank,row,userSpecifiedBank_charged,b_user_charged);
      }
    }

    ShowBank(resultBank, Logger::Header("CREATED BANK"));
  }

  int SectorFinder::GetSector(hipo::bank const& bank, int const pindex) const
  {
    int sector = -1;
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
