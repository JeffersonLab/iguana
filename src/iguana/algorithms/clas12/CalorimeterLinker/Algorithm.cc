#include "Algorithm.h"
#include "iguana/algorithms/TypeDefs.h"

namespace iguana::clas12 {

  REGISTER_IGUANA_ALGORITHM(CalorimeterLinker, "REC::Particle::Calorimeter");

  void CalorimeterLinker::Start(hipo::banklist& banks)
  {
    b_particle         = GetBankIndex(banks, "REC::Particle");
    b_calorimeter      = GetBankIndex(banks, "REC::Calorimeter");
    auto result_schema = CreateBank(banks, b_result, "REC::Particle::Calorimeter");
    i_pindex           = result_schema.getEntryOrder("pindex");
    i_pcal_found       = result_schema.getEntryOrder("pcal_found");
    i_pcal_sector      = result_schema.getEntryOrder("pcal_sector");
    i_pcal_lu          = result_schema.getEntryOrder("pcal_lu");
    i_pcal_lv          = result_schema.getEntryOrder("pcal_lv");
    i_pcal_lw          = result_schema.getEntryOrder("pcal_lw");
    i_pcal_energy      = result_schema.getEntryOrder("pcal_energy");
    i_ecin_found       = result_schema.getEntryOrder("ecin_found");
    i_ecin_sector      = result_schema.getEntryOrder("ecin_sector");
    i_ecin_lu          = result_schema.getEntryOrder("ecin_lu");
    i_ecin_lv          = result_schema.getEntryOrder("ecin_lv");
    i_ecin_lw          = result_schema.getEntryOrder("ecin_lw");
    i_ecin_energy      = result_schema.getEntryOrder("ecin_energy");
    i_ecout_found      = result_schema.getEntryOrder("ecout_found");
    i_ecout_sector     = result_schema.getEntryOrder("ecout_sector");
    i_ecout_lu         = result_schema.getEntryOrder("ecout_lu");
    i_ecout_lv         = result_schema.getEntryOrder("ecout_lv");
    i_ecout_lw         = result_schema.getEntryOrder("ecout_lw");
    i_ecout_energy     = result_schema.getEntryOrder("ecout_energy");
  }

  bool CalorimeterLinker::Run(hipo::banklist& banks) const
  {
    return Run(
        GetBank(banks, b_particle, "REC::Particle"),
        GetBank(banks, b_calorimeter, "REC::Calorimeter"),
        GetBank(banks, b_result, "REC::Particle::Calorimeter"));
  }

  bool CalorimeterLinker::Run(
      hipo::bank const& bank_particle,
      hipo::bank const& bank_calorimeter,
      hipo::bank& bank_result) const
  {
    bank_result.reset(); // IMPORTANT: always first `reset` the created bank(s)
    ShowBank(bank_particle, Logger::Header("INPUT PARTICLE BANK"));
    ShowBank(bank_calorimeter, Logger::Header("INPUT CALORIMETER BANK"));

    // sync new bank with particle bank, and fill it with zeroes
    bank_result.setRows(bank_particle.getRows());
    bank_result.getMutableRowList().setList(bank_particle.getRowList());
    for(int row = 0; row < bank_result.getRows(); row++) {
      bank_result.putShort(i_pindex, row, static_cast<int16_t>(row));
      bank_result.putByte(i_pcal_found, row, 0);
      bank_result.putInt(i_pcal_sector, row, 0);
      bank_result.putFloat(i_pcal_lu, row, 0);
      bank_result.putFloat(i_pcal_lv, row, 0);
      bank_result.putFloat(i_pcal_lw, row, 0);
      bank_result.putFloat(i_pcal_energy, row, 0);
      bank_result.putByte(i_ecin_found, row, 0);
      bank_result.putInt(i_ecin_sector, row, 0);
      bank_result.putFloat(i_ecin_lu, row, 0);
      bank_result.putFloat(i_ecin_lv, row, 0);
      bank_result.putFloat(i_ecin_lw, row, 0);
      bank_result.putFloat(i_ecin_energy, row, 0);
      bank_result.putByte(i_ecout_found, row, 0);
      bank_result.putInt(i_ecout_sector, row, 0);
      bank_result.putFloat(i_ecout_lu, row, 0);
      bank_result.putFloat(i_ecout_lv, row, 0);
      bank_result.putFloat(i_ecout_lw, row, 0);
      bank_result.putFloat(i_ecout_energy, row, 0);
    }

    // map particle `pindex` -> `CalorimeterLinkerVars` object
    std::map<int, CalorimeterLinkerVars> link_map;

    // loop over particle rows
    for(auto const& row_particle : bank_particle.getRowList()) {
      // create new `CalorimeterLinkerVars` object for this particle
      auto& link_particle = link_map[row_particle];
      // loop over `REC::Calorimeter` rows, setting elements of linked `CalorimeterLinkerVars`
      for(auto const& row_calorimeter : bank_calorimeter.getRowList()) {
        if(row_particle == bank_calorimeter.getShort("pindex", row_calorimeter)) {
          switch(bank_calorimeter.getByte("layer", row_calorimeter)) {
          case DetectorLayer::PCAL:
            link_particle.pcal_found  = 1;
            link_particle.pcal_sector = bank_calorimeter.getByte("sector", row_calorimeter);
            link_particle.pcal_lu     = bank_calorimeter.getFloat("lu", row_calorimeter);
            link_particle.pcal_lv     = bank_calorimeter.getFloat("lv", row_calorimeter);
            link_particle.pcal_lw     = bank_calorimeter.getFloat("lw", row_calorimeter);
            link_particle.pcal_energy = bank_calorimeter.getFloat("energy", row_calorimeter);
            break;
          case DetectorLayer::EC_INNER:
            link_particle.ecin_found  = 1;
            link_particle.ecin_sector = bank_calorimeter.getByte("sector", row_calorimeter);
            link_particle.ecin_lu     = bank_calorimeter.getFloat("lu", row_calorimeter);
            link_particle.ecin_lv     = bank_calorimeter.getFloat("lv", row_calorimeter);
            link_particle.ecin_lw     = bank_calorimeter.getFloat("lw", row_calorimeter);
            link_particle.ecin_energy = bank_calorimeter.getFloat("energy", row_calorimeter);
            break;
          case DetectorLayer::EC_OUTER:
            link_particle.ecout_found  = 1;
            link_particle.ecout_sector = bank_calorimeter.getByte("sector", row_calorimeter);
            link_particle.ecout_lu     = bank_calorimeter.getFloat("lu", row_calorimeter);
            link_particle.ecout_lv     = bank_calorimeter.getFloat("lv", row_calorimeter);
            link_particle.ecout_lw     = bank_calorimeter.getFloat("lw", row_calorimeter);
            link_particle.ecout_energy = bank_calorimeter.getFloat("energy", row_calorimeter);
            break;
          }
        }
      }
      // fill output bank
      bank_result.putByte(i_pcal_found, row_particle, link_particle.pcal_found);
      bank_result.putInt(i_pcal_sector, row_particle, link_particle.pcal_sector);
      bank_result.putFloat(i_pcal_lu, row_particle, link_particle.pcal_lu);
      bank_result.putFloat(i_pcal_lv, row_particle, link_particle.pcal_lv);
      bank_result.putFloat(i_pcal_lw, row_particle, link_particle.pcal_lw);
      bank_result.putFloat(i_pcal_energy, row_particle, link_particle.pcal_energy);
      bank_result.putByte(i_ecin_found, row_particle, link_particle.ecin_found);
      bank_result.putInt(i_ecin_sector, row_particle, link_particle.ecin_sector);
      bank_result.putFloat(i_ecin_lu, row_particle, link_particle.ecin_lu);
      bank_result.putFloat(i_ecin_lv, row_particle, link_particle.ecin_lv);
      bank_result.putFloat(i_ecin_lw, row_particle, link_particle.ecin_lw);
      bank_result.putFloat(i_ecin_energy, row_particle, link_particle.ecin_energy);
      bank_result.putByte(i_ecout_found, row_particle, link_particle.ecout_found);
      bank_result.putInt(i_ecout_sector, row_particle, link_particle.ecout_sector);
      bank_result.putFloat(i_ecout_lu, row_particle, link_particle.ecout_lu);
      bank_result.putFloat(i_ecout_lv, row_particle, link_particle.ecout_lv);
      bank_result.putFloat(i_ecout_lw, row_particle, link_particle.ecout_lw);
      bank_result.putFloat(i_ecout_energy, row_particle, link_particle.ecout_energy);
    }
    ShowBank(bank_result, Logger::Header("CREATED BANK"));
    return true;
  }

  void CalorimeterLinker::Stop()
  {
  }

}
