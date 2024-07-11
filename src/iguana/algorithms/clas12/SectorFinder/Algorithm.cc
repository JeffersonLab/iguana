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
    auto result_schema = CreateBank(banks, b_result, "REC::Particle::Sector", {"sector/I","pindex/I"}, 0xF000, 4);
    i_sector = result_schema.getEntryOrder("sector");
    i_pindex = result_schema.getEntryOrder("pindex");
  }

  void SectorFinder::Run(hipo::banklist& banks) const
  {
    auto& particleBank = GetBank(banks, b_particle, "REC::Particle");
    auto& resultBank   = GetBank(banks, b_result, "REC::Particle::Sector");

    std::vector<int> sectors_uncharged;
    std::vector<int> pindices_uncharged;
    if(userSpecifiedBank_uncharged){
      auto const& userBank = GetBank(banks, b_user_uncharged);
      GetListsSectorPindex(userBank,sectors_uncharged,pindices_uncharged);
    }

    std::vector<int> sectors_charged;
    std::vector<int> pindices_charged;
    if(userSpecifiedBank_charged){
      auto const& userBank = GetBank(banks, b_user_charged);
      GetListsSectorPindex(userBank,sectors_charged,pindices_charged);
    }

    std::vector<int> sectors_track;
    std::vector<int> pindices_track;
    if(!userSpecifiedBank_charged || !userSpecifiedBank_uncharged){
      auto const& trackBank = GetBank(banks, b_track);
      GetListsSectorPindex(trackBank,sectors_track,pindices_track);
    }

    std::vector<int> sectors_cal;
    std::vector<int> pindices_cal;
    if(!userSpecifiedBank_charged || !userSpecifiedBank_uncharged){
      auto const& calBank = GetBank(banks, b_calorimeter);
      GetListsSectorPindex(calBank,sectors_cal,pindices_cal);
    }

    std::vector<int> sectors_scint;
    std::vector<int> pindices_scint;
    if(!userSpecifiedBank_charged || !userSpecifiedBank_uncharged){
      auto const& scintBank = GetBank(banks, b_scint);
      GetListsSectorPindex(scintBank,sectors_scint,pindices_scint);
    }
    
    
    // sync new bank with particle bank, and fill it with zeroes
    resultBank.setRows(particleBank.getRows());
    resultBank.getMutableRowList().setList(particleBank.getRowList());
    for(int row = 0; row < resultBank.getRows(); row++){
      resultBank.putInt(i_sector, row, 0);
      resultBank.putInt(i_pindex, row, row);
    }


    for(int row = 0; row < particleBank.getRows(); row++) {
      int charge=particleBank.getInt("charge",row);
      bool userSp=false;
      //can't have reference to nothing
      std::vector<int>& sct_us=sectors_uncharged;
      std::vector<int>& pin_us=pindices_uncharged;
      if(charge==0){
        userSp=userSpecifiedBank_uncharged;
        sct_us=sectors_uncharged;
        pin_us=pindices_uncharged;
      } else {
        userSp=userSpecifiedBank_charged;
        sct_us=sectors_charged;
        pin_us=pindices_charged;
      }
      
      if(userSp){
        int sect=GetSector(sct_us, pin_us,row);
        if (sect!=-1){
          resultBank.putInt(i_sector, row, sect);
          resultBank.putInt(i_pindex, row, row);
        }
      } else {
        int trackSector=GetSector(sectors_track, pindices_track,row);
        int calSector=GetSector(sectors_cal, pindices_cal,row);
        int scintSector=GetSector(sectors_scint, pindices_scint,row);
        if(trackSector != -1) {
          //std::cout<<"track pindex "<<row<<" sect "<<trackSector<<std::endl;
          resultBank.putInt(i_sector, row, trackSector);
          resultBank.putInt(i_pindex, row, row);
        }
        else if(scintSector != -1) {
          //std::cout<<"scint pindex "<<row<<" sect "<<scintSector<<std::endl;
          resultBank.putInt(i_sector, row, scintSector);
          resultBank.putInt(i_pindex, row, row);
        }
        else if(calSector != -1){
          //std::cout<<"cal pindex "<<row<<" sect "<<calSector<<std::endl;
          resultBank.putInt(i_sector, row, calSector);
          resultBank.putInt(i_pindex, row, row);
        }
      }
    }

    ShowBank(resultBank, Logger::Header("CREATED BANK"));
  }

  void SectorFinder::GetListsSectorPindex(hipo::bank const& bank, std::vector<int>& sectors, std::vector<int>& pindices) const
  {
    for(auto const& row : bank.getRowList()) {
      //check that we're only using FD detectors
      //eg have "sectors" in CND which we don't want to add here
      int det=bank.getInt("detector",row);
      std::set<int>::iterator it = listFDDets.find(det);
      if (it != listFDDets.end()) {
        sectors.push_back(bank.getInt("sector", row));
        pindices.push_back(bank.getInt("pindex", row));
      }
    }
  }
  
  int SectorFinder::GetSector(std::vector<int> const& sectors, std::vector<int> const& pindices, int const pindex) const
  {
    for(long unsigned int i=0;i<sectors.size();i++){
      if (pindices.at(i)==pindex){
        return sectors.at(i);
      }
    }
    return -1;
  }

  void SectorFinder::Stop()
  {
  }

}
