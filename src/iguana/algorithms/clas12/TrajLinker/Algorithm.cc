#include "Algorithm.h"

namespace iguana::clas12 {

  REGISTER_IGUANA_ALGORITHM(TrajLinker, "REC::Particle::Traj");

  void TrajLinker::Start(hipo::banklist& banks)
  {
    b_particle         = GetBankIndex(banks,"REC::Particle");
    b_traj             = GetBankIndex(banks,"REC::Traj");
    auto result_schema = CreateBank(banks, b_result,"REC::Particle::Traj");
    i_pindex           = result_schema.getEntryOrder("pindex");
    i_sector           = result_schema.getEntryOrder("sector");
    i_r1x              = result_schema.getEntryOrder("r1x");
    i_r1y              = result_schema.getEntryOrder("r1y");
    i_r1z              = result_schema.getEntryOrder("r1z");
    i_r2x              = result_schema.getEntryOrder("r2x");
    i_r2y              = result_schema.getEntryOrder("r2y");
    i_r2z              = result_schema.getEntryOrder("r2z");
    i_r3x              = result_schema.getEntryOrder("r3x");
    i_r3y              = result_schema.getEntryOrder("r3y");
    i_r3z              = result_schema.getEntryOrder("r3z");
  }

  void TrajLinker::Run(hipo::banklist& banks) const
  {
    auto& bank_particle = GetBank(banks, b_particle, "REC::Particle");
    auto& bank_traj     = GetBank(banks, b_traj, "REC::Traj");
    auto& bank_result   = GetBank(banks, b_result, "REC::Particle::Traj");

    // sync new bank with particle bank, and fill it with zeroes
    bank_result.setRows(bank_particle.getRows());
    bank_result.getMutableRowList().setList(bank_particle.getRowList());
    for(int row = 0; row < bank_result.getRows(); row++){
      for(int ent = 0; ent < bank_result.getSchema().getEntries(); ent++) {
        bank_result.putShort(i_pindex, row, static_cast<int16_t>(row));
        bank_result.putInt(i_sector, row, 0);
        bank_result.putFloat(i_r1x, row, 0);
        bank_result.putFloat(i_r1y, row, 0);
        bank_result.putFloat(i_r1z, row, 0);
        bank_result.putFloat(i_r2x, row, 0);
        bank_result.putFloat(i_r2y, row, 0);
        bank_result.putFloat(i_r2z, row, 0);
        bank_result.putFloat(i_r3x, row, 0);
        bank_result.putFloat(i_r3y, row, 0);
        bank_result.putFloat(i_r3z, row, 0);
      }
    }

    // map particle `pindex` -> `TrajLinkerVars` object
    std::map<int, TrajLinkerVars> link_map;

    // loop over particle rows
    for(auto const& row_particle : bank_particle.getRowList()) {
      // create new `TrajLinkerVars` object for this particle
      auto& link_particle = link_map[row_particle];
      // loop over `REC::Traj` rows, setting elements of linked `TrajLinkerVars`
      for(auto const& row_traj : bank_traj.getRowList()) {
        if(row_particle == bank_traj.getShort("pindex", row_traj)) {
          auto x     = bank_traj.getFloat("x", row_traj);
          auto y     = bank_traj.getFloat("y", row_traj);
          auto z     = bank_traj.getFloat("z", row_traj);
          auto layer = bank_traj.getInt("layer", row_traj);
          switch(layer){
            case 6: // region 1
              link_particle.r1x = x;
              link_particle.r1y = y;
              link_particle.r1z = z;
              break;
            case 18: // region 2
              link_particle.r2x = x;
              link_particle.r2y = y;
              link_particle.r2z = z;
              // Determine Sector from the center of the DC
              link_particle.sector = GetSector(link_particle.r2x, link_particle.r2y, link_particle.r2z);
              break;
            case 36: // region 3
              link_particle.r3x = x;
              link_particle.r3y = y;
              link_particle.r3z = z;
              break;
          }
        }
      }
      // fill output bank
      bank_result.putInt(i_sector, row_particle, link_particle.sector);
      bank_result.putFloat(i_r1x, row_particle, link_particle.r1x);
      bank_result.putFloat(i_r1y, row_particle, link_particle.r1y);
      bank_result.putFloat(i_r1z, row_particle, link_particle.r1z);
      bank_result.putFloat(i_r2x, row_particle, link_particle.r2x);
      bank_result.putFloat(i_r2y, row_particle, link_particle.r2y);
      bank_result.putFloat(i_r2z, row_particle, link_particle.r2z);
      bank_result.putFloat(i_r3x, row_particle, link_particle.r3x);
      bank_result.putFloat(i_r3y, row_particle, link_particle.r3y);
      bank_result.putFloat(i_r3z, row_particle, link_particle.r3z);
    }
    ShowBank(bank_result, Logger::Header("CREATED BANK"));
  }
  void TrajLinker::Stop()
  {
  }

  int TrajLinker::GetSector(float const& x, float const& y, float const& z) const
  {
    float phi = 180 / M_PI * atan2(y / sqrt(pow(x,2) + pow(y,2) + pow(z,2)),
        x /sqrt(pow(x,2) + pow(y,2) + pow(z,2)));
    if(phi<30 && phi>=-30) return 1;
    else if(phi<90 && phi>=30) return 2;
    else if(phi<150 && phi>=90) return 3;
    else if(phi>=150 || phi<-150) return 4;
    else if(phi<-90 && phi>=-150) return 5;
    else if(phi<-30 && phi>=-90) return 6;
    return -1;
  }

}
