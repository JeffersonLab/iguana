#include "Algorithm.h"

namespace iguana::clas12 {

  REGISTER_IGUANA_ALGORITHM(CalorimeterLinker, "REC::Particle::Calorimeter");

  void CalorimeterLinker::Start(hipo::banklist& banks)
  {

    b_particle         = GetBankIndex(banks,"REC::Particle");
    b_calorimeter      = GetBankIndex(banks,"REC::Calorimeter");
    auto result_schema = CreateBank(banks, b_result,"REC::Particle::Calorimeter");
    i_pindex           = result_schema.getEntryOrder("pindex");
    i_pcal_sector      = result_schema.getEntryOrder("pcal_sector");
    i_pcal_lu          = result_schema.getEntryOrder("pcal_lu");
    i_pcal_lv          = result_schema.getEntryOrder("pcal_lv");
    i_pcal_lw          = result_schema.getEntryOrder("pcal_lw");
    i_ecin_sector      = result_schema.getEntryOrder("ecin_sector");
    i_ecin_lu          = result_schema.getEntryOrder("ecin_lu");
    i_ecin_lv          = result_schema.getEntryOrder("ecin_lv");
    i_ecin_lw          = result_schema.getEntryOrder("ecin_lw");
    i_ecout_sector     = result_schema.getEntryOrder("ecout_sector");
    i_ecout_lu         = result_schema.getEntryOrder("ecout_lu");
    i_ecout_lv         = result_schema.getEntryOrder("ecout_lv");
    i_ecout_lw         = result_schema.getEntryOrder("ecout_lw");
  }

  void CalorimeterLinker::Run(hipo::banklist& banks) const
  {
    auto& bank_particle    = GetBank(banks, b_particle, "REC::Particle");
    auto& bank_calorimeter = GetBank(banks, b_calorimeter, "REC::Calorimeter");
    auto& bank_result      = GetBank(banks, b_result, "REC::Particle::Calorimeter");

    // sync new bank with particle bank, and fill it with zeroes
    bank_result.setRows(bank_particle.getRows());
    bank_result.getMutableRowList().setList(bank_particle.getRowList());
    for(int row = 0; row < bank_result.getRows(); row++){
      for(int ent = 0; ent < bank_result.getSchema().getEntries(); ent++) {
        bank_result.putShort(i_pindex, row, static_cast<int16_t>(row));
        bank_result.putInt(i_pcal_sector, row, 0);
        bank_result.putFloat(i_pcal_lu, row, 0);
        bank_result.putFloat(i_pcal_lv, row, 0);
        bank_result.putFloat(i_pcal_lw, row, 0);
        bank_result.putInt(i_ecin_sector, row, 0);
        bank_result.putFloat(i_ecin_lu, row, 0);
        bank_result.putFloat(i_ecin_lv, row, 0);
        bank_result.putFloat(i_ecin_lw, row, 0);
        bank_result.putInt(i_ecout_sector, row, 0);
        bank_result.putFloat(i_ecout_lu, row, 0);
        bank_result.putFloat(i_ecout_lv, row, 0);
        bank_result.putFloat(i_ecout_lw, row, 0);
      }
    }

    // map particle `pindex` -> `CalorimeterLinkerVars` object
    std::map<int, CalorimeterLinkerVars> link_map;

    // loop over particle rows
    for(auto const& row_particle : bank_particle.getRowList()) {
      // create new `CalorimeterLinkerVars` object for this `pindex`
      auto pindex         = bank_particle.getShort("pindex", row_particle);
      auto& link_particle = link_map[pindex];
      // loop over `REC::Calorimeter` rows, setting elements of linked `CalorimeterLinkerVars`
      for(auto const& row_calorimeter : bank_calorimeter.getRowList()) {
        if(pindex == bank_calorimeter.getShort("pindex", row_calorimeter)) {
          auto sector = bank_calorimeter.getByte("sector", row_calorimeter);
          auto lu     = bank_calorimeter.getFloat("lu", row_calorimeter);
          auto lv     = bank_calorimeter.getFloat("lv", row_calorimeter);
          auto lw     = bank_calorimeter.getFloat("lw", row_calorimeter);
          auto layer  = bank_calorimeter.getByte("layer", row_calorimeter);
          switch(layer){
            case 1:
              link_particle.pcal_sector = sector;
              link_particle.pcal_lu     = lu;
              link_particle.pcal_lv     = lv;
              link_particle.pcal_lw     = lw;
              break;
            case 4:
              link_particle.ecin_sector = sector;
              link_particle.ecin_lu     = lu;
              link_particle.ecin_lv     = lv;
              link_particle.ecin_lw     = lw;
              break;
            case 7:
              link_particle.ecout_sector = sector;
              link_particle.ecout_lu     = lu;
              link_particle.ecout_lv     = lv;
              link_particle.ecout_lw     = lw;
              break;
          }
        }
      }
      // fill output bank
      bank_result.putInt(i_pcal_sector, row_particle, link_particle.pcal_sector);
      bank_result.putFloat(i_pcal_lu, row_particle, link_particle.pcal_lu);
      bank_result.putFloat(i_pcal_lv, row_particle, link_particle.pcal_lv);
      bank_result.putFloat(i_pcal_lw, row_particle, link_particle.pcal_lw);
      bank_result.putInt(i_ecin_sector, row_particle, link_particle.ecin_sector);
      bank_result.putFloat(i_ecin_lu, row_particle, link_particle.ecin_lu);
      bank_result.putFloat(i_ecin_lv, row_particle, link_particle.ecin_lv);
      bank_result.putFloat(i_ecin_lw, row_particle, link_particle.ecin_lw);
      bank_result.putInt(i_ecout_sector, row_particle, link_particle.ecout_sector);
      bank_result.putFloat(i_ecout_lu, row_particle, link_particle.ecout_lu);
      bank_result.putFloat(i_ecout_lv, row_particle, link_particle.ecout_lv);
      bank_result.putFloat(i_ecout_lw, row_particle, link_particle.ecout_lw);
    }
    ShowBank(bank_result, Logger::Header("CREATED BANK"));
  }

  void CalorimeterLinker::Stop()
  {
  }

}
