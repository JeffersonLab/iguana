#include "SectorFinder.h"

namespace iguana::clas12 {

  REGISTER_IGUANA_ALGORITHM(SectorFinder);

  void SectorFinder::Start(hipo::banklist& banks)
  {

    // define options, their default values, and cache them
    ParseYAMLConfig();
    o_bankname = GetOptionScalar<std::string>("bank");

    // get expected bank indices
    b_particle = GetBankIndex(banks, "REC::Particle");
    if(o_bankname!="default"){
        b_nondefault=GetBankIndex(banks, o_bankname);
        b_calorimeter=0;
        b_track=0;
        b_scint=0;

    } else{
        b_calorimeter=GetBankIndex(banks, "REC::Calorimeter");
        b_track=GetBankIndex(banks, "REC::Track");
        b_scint=GetBankIndex(banks, "REC::Scintillator");
        b_nondefault=0;
    }
  }


  //explicitly leave empty, run does nothing
  void SectorFinder::Run(hipo::banklist& banks) const
  {
  }

  int SectorFinder::getSector(hipo::bank &bank, int pindex) const {
    int sector=0;
    for(int row = 0; row < bank.getRows(); row++) {
      if(bank.getInt("pindex",row)==pindex){
        return bank.getInt("sector",row);
      }
    }
    return sector;
  }

  std::vector<int> SectorFinder::Find(hipo::banklist& banks) const
  {
    std::vector<int> sectors;
    // get the banks
    auto& particleBank = GetBank(banks, b_particle, "REC::Particle");
    auto& nondefaultBank=GetBank(banks, b_nondefault);
    auto& calBank=GetBank(banks, b_calorimeter);
    auto& scintBank=GetBank(banks,b_scint);
    auto& trackBank=GetBank(banks,b_track);

    // filter the input bank for requested PDG code(s)
    for(int row = 0; row < particleBank.getRows(); row++) {
      if(o_bankname!="default"){
        //if user specified a secific bank
        if(nondefaultBank.getRows()>0){
          sectors.push_back(getSector(nondefaultBank,row));
        } else{
          //bank may be empty
          sectors.push_back(0);
        } 
      } else{
          if(trackBank.getRows()>0){
            sectors.push_back(getSector(trackBank,row));
          } else if(scintBank.getRows()>0){
            sectors.push_back(getSector(scintBank,row));
          } else if(calBank.getRows()>0){
            sectors.push_back(getSector(calBank,row));
          } else{
            //banks may be empty
            sectors.push_back(0);
          }
          
        }
    }

    return sectors;
  }


  void SectorFinder::Stop()
  {
  }

}
