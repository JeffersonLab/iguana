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
    auto result_schema = CreateBank(banks, b_result, "REC::Particle::Sector", {"sector/I","pindex/S"}, 0xF000, 4);
    i_sector = result_schema.getEntryOrder("sector");
    i_pindex = result_schema.getEntryOrder("pindex");
  }

  void SectorFinder::Run(hipo::banklist& banks) const
  {
    auto& particleBank = GetBank(banks, b_particle, "REC::Particle");
    auto& resultBank   = GetBank(banks, b_result, "REC::Particle::Sector");

    std::vector<int> sectors_user_uncharged;
    std::vector<int> pindices_user_uncharged;
    if(userSpecifiedBank_uncharged){
      auto const& userBank = GetBank(banks, b_user_uncharged);
      GetListsSectorPindex(userBank,sectors_user_uncharged,pindices_user_uncharged);
    }

    std::vector<int> sectors_user_charged;
    std::vector<int> pindices_user_charged;
    if(userSpecifiedBank_charged){
      auto const& userBank = GetBank(banks, b_user_charged);
      GetListsSectorPindex(userBank,sectors_user_charged,pindices_user_charged);
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
      resultBank.putShort(i_pindex, row, static_cast<int16_t>(row));
    }


    for(int row = 0; row < particleBank.getRows(); row++) {

      auto charge=particleBank.getInt("charge",row);
      int sect = -1;

      // if user-specified bank
      if(charge==0 ? userSpecifiedBank_uncharged : userSpecifiedBank_charged)
        sect = GetSector(
            charge==0 ? sectors_user_uncharged : sectors_user_charged,
            charge==0 ? pindices_user_uncharged : pindices_user_charged,
            row);
      else // if not user-specified bank, use the standard method
        sect = GetStandardSector(
            sectors_track,
            pindices_track,
            sectors_cal,
            pindices_cal,
            sectors_scint,
            pindices_scint,
            row);

      if (sect!=-1){
        resultBank.putInt(i_sector, row, sect);
        resultBank.putShort(i_pindex, row, static_cast<int16_t>(row));
      }
    }

    ShowBank(resultBank, Logger::Header("CREATED BANK"));
  }

  void SectorFinder::GetListsSectorPindex(hipo::bank const& bank, std::vector<int>& sectors, std::vector<int>& pindices) const
  {
    for(auto const& row : bank.getRowList()) {
      //check that we're only using FD detectors
      //eg have "sectors" in CND which we don't want to add here
      auto det=bank.getByte("detector",row);
      if (listFDDets.find(det) != listFDDets.end()) {
        sectors.push_back(bank.getInt("sector", row));
        pindices.push_back(bank.getShort("pindex", row));
      }
    }
  }

  int SectorFinder::GetSector(std::vector<int> const& sectors, std::vector<int> const& pindices, int const& pindex_particle) const
  {
    for(std::size_t i=0;i<sectors.size();i++){
      if (pindices.at(i)==pindex_particle){
        return sectors.at(i);
      }
    }
    return -1;
  }

  int SectorFinder::GetStandardSector(
      std::vector<int> const& sectors_track,
      std::vector<int> const& pindices_track,
      std::vector<int> const& sectors_cal,
      std::vector<int> const& pindices_cal,
      std::vector<int> const& sectors_scint,
      std::vector<int> const& pindices_scint,
      int const& pindex_particle) const
  {
    enum det_enum {kTrack, kScint, kCal, nDet}; // try to get sector from these detectors, in this order
    for(int d = 0; d < nDet; d++) {
      int sect = -1;
      std::string det_name;
      switch(d) {
        case kTrack:
          sect = GetSector(sectors_track, pindices_track, pindex_particle);
          det_name = "track";
          break;
        case kScint:
          sect = GetSector(sectors_scint, pindices_scint, pindex_particle);
          det_name = "scint";
          break;
        case kCal:
          sect = GetSector(sectors_cal, pindices_cal, pindex_particle);
          det_name = "cal";
          break;
      }
      if(sect != -1) {
        m_log->Trace("{} pindex {} sect {}", det_name,  pindex_particle, sect);
        return sect;
      }
    }
    return -1;
  }

  std::vector<int> SectorFinder::GetStandardSectors(
      std::vector<int> const& sectors_track,
      std::vector<int> const& pindices_track,
      std::vector<int> const& sectors_cal,
      std::vector<int> const& pindices_cal,
      std::vector<int> const& sectors_scint,
      std::vector<int> const& pindices_scint,
      std::vector<int> const& pindices_particle) const
  {
    std::vector<int> sect_list;
    for(auto const& pindex : pindices_particle)
      sect_list.push_back(GetStandardSector(
      sectors_track,
      pindices_track,
      sectors_cal,
      pindices_cal,
      sectors_scint,
      pindices_scint,
      pindex));
    return sect_list;
  }

  void SectorFinder::Stop()
  {
  }

}
