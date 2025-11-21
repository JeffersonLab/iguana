#include "Algorithm.h"

namespace iguana::clas12 {

  REGISTER_IGUANA_ALGORITHM(SectorFinder, "REC::Particle::Sector");

  void SectorFinder::Start(hipo::banklist& banks)
  {

    // define options, their default values, and cache them
    ParseYAMLConfig();
    o_bankname_charged = GetOptionScalar<std::string>("bank_charged");
    try {
      o_bankname_neutral = GetOptionScalar<std::string>("bank_neutral");
    }
    catch(std::runtime_error const& ex) {
      m_log->Warn("searching instead for configuration parameter named 'bank_uncharged'...");
      o_bankname_neutral = GetOptionScalar<std::string>("bank_uncharged");
      m_log->Warn("...found 'bank_uncharged' and using it; note that 'bank_uncharged' has been renamed to 'bank_neutral', please update your configuration");
    }

    bool setDefaultBanks = false;
    // get expected bank indices
    b_particle = GetBankIndex(banks, "REC::Particle");
    if(o_bankname_charged != "default") {
      b_user_charged            = GetBankIndex(banks, o_bankname_charged);
      userSpecifiedBank_charged = true;
    }
    else {
      b_track                   = GetBankIndex(banks, "REC::Track");
      b_calorimeter             = GetBankIndex(banks, "REC::Calorimeter");
      b_scint                   = GetBankIndex(banks, "REC::Scintillator");
      setDefaultBanks           = true;
      userSpecifiedBank_charged = false;
    }

    if(o_bankname_neutral != "default") {
      b_user_neutral            = GetBankIndex(banks, o_bankname_neutral);
      userSpecifiedBank_neutral = true;
    }
    else {
      // avoid setting default banks twice
      if(!setDefaultBanks) {
        b_track         = GetBankIndex(banks, "REC::Track");
        b_calorimeter   = GetBankIndex(banks, "REC::Calorimeter");
        b_scint         = GetBankIndex(banks, "REC::Scintillator");
        setDefaultBanks = true;
      }
      userSpecifiedBank_neutral = false;
    }

    // create the output bank
    auto result_schema = CreateBank(banks, b_result, "REC::Particle::Sector");
    i_sector           = result_schema.getEntryOrder("sector");
    i_pindex           = result_schema.getEntryOrder("pindex");
  }

  bool SectorFinder::Run(hipo::banklist& banks) const
  {
    auto includeDefaultBanks = !(userSpecifiedBank_charged && userSpecifiedBank_neutral);
    return RunImpl(
        &GetBank(banks, b_particle, "REC::Particle"),
        includeDefaultBanks ? &GetBank(banks, b_track, "REC::Track") : nullptr,
        includeDefaultBanks ? &GetBank(banks, b_calorimeter, "REC::Calorimeter") : nullptr,
        includeDefaultBanks ? &GetBank(banks, b_scint, "REC::Scintillator") : nullptr,
        userSpecifiedBank_charged ? &GetBank(banks, b_user_charged) : nullptr,
        userSpecifiedBank_neutral ? &GetBank(banks, b_user_neutral) : nullptr,
        &GetBank(banks, b_result, "REC::Particle::Sector"));
  }

  bool SectorFinder::RunImpl(
      hipo::bank const* particleBank,
      hipo::bank const* trackBank,
      hipo::bank const* calBank,
      hipo::bank const* scintBank,
      hipo::bank const* userChargedBank,
      hipo::bank const* userNeutralBank,
      hipo::bank* resultBank) const
  {
    resultBank->reset(); // IMPORTANT: always first `reset` the created bank(s)

    std::vector<int> sectors_track;
    std::vector<int> pindices_track;
    std::vector<int> sectors_cal;
    std::vector<int> pindices_cal;
    std::vector<int> sectors_scint;
    std::vector<int> pindices_scint;
    std::vector<int> sectors_user_neutral;
    std::vector<int> pindices_user_neutral;
    std::vector<int> sectors_user_charged;
    std::vector<int> pindices_user_charged;

    if(!userSpecifiedBank_charged || !userSpecifiedBank_neutral) {
      if(trackBank != nullptr && calBank != nullptr && scintBank != nullptr) {
        GetListsSectorPindex(*trackBank, sectors_track, pindices_track);
        GetListsSectorPindex(*scintBank, sectors_scint, pindices_scint);
        GetListsSectorPindex(*calBank, sectors_cal, pindices_cal);
      }
      else
        throw std::runtime_error("SectorFinder::RunImpl called with unexpected null pointer to either the track, calorimeter, or scintillator bank(s); please contact the maintainers");
    }

    if(userSpecifiedBank_neutral) {
      if(userNeutralBank != nullptr)
        GetListsSectorPindex(*userNeutralBank, sectors_user_neutral, pindices_user_neutral);
      else
        throw std::runtime_error("SectorFinder::RunImpl called with unexpected null pointer to a user-specified bank; please contact the maintainers");
    }

    if(userSpecifiedBank_charged) {
      if(userChargedBank != nullptr)
        GetListsSectorPindex(*userChargedBank, sectors_user_charged, pindices_user_charged);
      else
        throw std::runtime_error("SectorFinder::RunImpl called with unexpected null pointer to a user-specified bank; please contact the maintainers");
    }

    // trace logging
    if(m_log->GetLevel() <= Logger::Level::trace) {
      m_log->Trace("pindices_track = {}", fmt::join(pindices_track, ","));
      m_log->Trace("sectors_track  = {}", fmt::join(sectors_track, ","));
      m_log->Trace("pindices_scint = {}", fmt::join(pindices_scint, ","));
      m_log->Trace("sectors_scint  = {}", fmt::join(sectors_scint, ","));
      m_log->Trace("pindices_cal   = {}", fmt::join(pindices_cal, ","));
      m_log->Trace("sectors_cal    = {}", fmt::join(sectors_cal, ","));
      m_log->Trace("pindices_user_neutral = {}", fmt::join(pindices_user_neutral, ","));
      m_log->Trace("sectors_user_neutral  = {}", fmt::join(sectors_user_neutral, ","));
      m_log->Trace("pindices_user_charged = {}", fmt::join(pindices_user_charged, ","));
      m_log->Trace("sectors_user_charged  = {}", fmt::join(sectors_user_charged, ","));
    }

    // sync new bank with particle bank
    resultBank->setRows(particleBank->getRows());
    resultBank->getMutableRowList().setList(particleBank->getRowList());

    // some downstream algorithms may still need sector info, so obtain sector for _all_ particles,
    // not just the ones that were filtered out (use `.getRows()` rather than `.getRowList()`)
    for(int row = 0; row < particleBank->getRows(); row++) {

      auto charge = particleBank->getInt("charge", row);
      int sect    = UNKNOWN_SECTOR;

      // if user-specified bank
      if(charge == 0 ? userSpecifiedBank_neutral : userSpecifiedBank_charged)
        sect = GetSector(
            charge == 0 ? sectors_user_neutral : sectors_user_charged,
            charge == 0 ? pindices_user_neutral : pindices_user_charged,
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

      resultBank->putInt(i_sector, row, sect);
      resultBank->putShort(i_pindex, row, static_cast<int16_t>(row));
    }

    ShowBank(*resultBank, Logger::Header("CREATED BANK"));
    return true;
  }

  void SectorFinder::GetListsSectorPindex(hipo::bank const& bank, std::vector<int>& sectors, std::vector<int>& pindices) const
  {
    if(m_log->GetLevel() <= Logger::Level::trace) {
      m_log->Trace("called `GetListsSectorPindex` for the following bank:");
      bank.show();
    }
    for(auto const& row : bank.getRowList()) {
      // check that we're only using FD detectors
      // eg have "sectors" in CND which we don't want to add here
      auto det = bank.getByte("detector", row);
      if(listFDDets.find(det) != listFDDets.end()) {
        sectors.push_back(bank.getInt("sector", row));
        pindices.push_back(bank.getShort("pindex", row));
      }
    }
  }

  int SectorFinder::GetSector(std::vector<int> const& sectors, std::vector<int> const& pindices, int const& pindex_particle) const
  {
    for(std::size_t i = 0; i < sectors.size(); i++) {
      if(pindices.at(i) == pindex_particle) {
        auto sect = sectors.at(i);
        return IsValidSector(sect) ? sect : UNKNOWN_SECTOR;
      }
    }
    return UNKNOWN_SECTOR; // pindex not found
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
    enum det_enum { kTrack,
                    kScint,
                    kCal,
                    nDet }; // try to get sector from these detectors, in this order
    for(int d = 0; d < nDet; d++) {
      int sect = UNKNOWN_SECTOR;
      std::string det_name;
      switch(d) {
      case kTrack:
        sect     = GetSector(sectors_track, pindices_track, pindex_particle);
        det_name = "track";
        break;
      case kScint:
        sect     = GetSector(sectors_scint, pindices_scint, pindex_particle);
        det_name = "scint";
        break;
      case kCal:
        sect     = GetSector(sectors_cal, pindices_cal, pindex_particle);
        det_name = "cal";
        break;
      }
      m_log->Trace("{} pindex {} sect {}", det_name, pindex_particle, sect);
      if(IsValidSector(sect)) // return this sector number; if not valid, continue to next detector in `det_enum`
        return sect;
    }
    return UNKNOWN_SECTOR; // not found in any detector in `det_enum`
  }

  std::vector<int> SectorFinder::GetStandardSector(
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
