#include "SectorFinder.h"
#include <cassert>

namespace iguana::clas12 {

  REGISTER_IGUANA_ALGORITHM(SectorFinder);

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
  }


  // explicitly leave empty, run does nothing
  void SectorFinder::Run(hipo::banklist& banks) const
  {
  }

  int SectorFinder::getSector(hipo::bank& bank, int pindex) const
  {
    int sector = 0;
    for(int row = 0; row < bank.getRows(); row++) {
      if(bank.getInt("pindex", row) == pindex) {
        return bank.getInt("sector", row);
      }
    }
    return sector;
  }

  std::vector<int> SectorFinder::Find(hipo::banklist& banks) const
  {
    std::vector<int> sectors;
    auto& particleBank = GetBank(banks, b_particle, "REC::Particle");

    // filter the input bank for requested PDG code(s)
    if(userSpecifiedBank) { // if user specified a specific bank
      auto& userBank = GetBank(banks, b_user);
      for(int row = 0; row < particleBank.getRows(); row++) {
        if(userBank.getRows() > 0) {
          sectors.push_back(getSector(userBank, row));
        }
        else {
          // bank may be empty
          sectors.push_back(0);
        }
      }
    }
    else { // use the standard method
      auto& calBank   = GetBank(banks, b_calorimeter);
      auto& scintBank = GetBank(banks, b_scint);
      auto& trackBank = GetBank(banks, b_track);
      for(int row = 0; row < particleBank.getRows(); row++) {
        int trackSector = 0;
        if(trackBank.getRows() > 0) {
          trackSector = getSector(trackBank, row);
        }

        int scintSector = 0;
        if(scintBank.getRows() > 0) {
          scintSector = getSector(scintBank, row);
        }

        int calSector = 0;
        if(calBank.getRows() > 0) {
          calSector = getSector(calBank, row);
        }

        if(trackSector != 0) {
          sectors.push_back(trackSector);
        }
        else if(scintSector != 0) {
          sectors.push_back(scintSector);
        }
        else {
          // add even if calSector is 0
          // need an entry per pindex
          // can happen that particle not in FD
          // ie sector is 0
          sectors.push_back(calSector);
        }
      }
    }

    // verify results have the same size as `particleBank`
    assert((sectors.size() == static_cast<decltype(sectors)::size_type>(particleBank.getRows())));

    return sectors;
  }


  void SectorFinder::Stop()
  {
  }

}
