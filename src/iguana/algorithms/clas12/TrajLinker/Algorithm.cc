#include "Algorithm.h"
#include "iguana/algorithms/TypeDefs.h"

namespace iguana::clas12 {

  REGISTER_IGUANA_ALGORITHM(TrajLinker, "REC::Particle::Traj");

  void TrajLinker::Start(hipo::banklist& banks)
  {
    b_particle         = GetBankIndex(banks,"REC::Particle");
    b_traj             = GetBankIndex(banks,"REC::Traj");
    auto result_schema = CreateBank(banks, b_result,"REC::Particle::Traj");
    i_pindex           = result_schema.getEntryOrder("pindex");
    i_sector           = result_schema.getEntryOrder("sector");
    i_r1_found         = result_schema.getEntryOrder("r1_found");
    i_r1_x             = result_schema.getEntryOrder("r1_x");
    i_r1_y             = result_schema.getEntryOrder("r1_y");
    i_r1_z             = result_schema.getEntryOrder("r1_z");
    i_r2_found         = result_schema.getEntryOrder("r2_found");
    i_r2_x             = result_schema.getEntryOrder("r2_x");
    i_r2_y             = result_schema.getEntryOrder("r2_y");
    i_r2_z             = result_schema.getEntryOrder("r2_z");
    i_r3_found         = result_schema.getEntryOrder("r3_found");
    i_r3_x             = result_schema.getEntryOrder("r3_x");
    i_r3_y             = result_schema.getEntryOrder("r3_y");
    i_r3_z             = result_schema.getEntryOrder("r3_z");
  }

  void TrajLinker::Run(hipo::banklist& banks) const
  {
    auto& bank_particle = GetBank(banks, b_particle, "REC::Particle");
    auto& bank_traj     = GetBank(banks, b_traj, "REC::Traj");
    auto& bank_result   = GetBank(banks, b_result, "REC::Particle::Traj");

    ShowBank(bank_particle, Logger::Header("INPUT PARTICLE BANK"));
    ShowBank(bank_traj, Logger::Header("INPUT TRAJECTORY BANK"));

    // sync new bank with particle bank, and fill it with zeroes
    bank_result.setRows(bank_particle.getRows());
    bank_result.getMutableRowList().setList(bank_particle.getRowList());
    for(int row = 0; row < bank_result.getRows(); row++) {
      bank_result.putShort(i_pindex, row, static_cast<int16_t>(row));
      bank_result.putInt(i_sector, row, 0);
      bank_result.putByte(i_r1_found, row, 0);
      bank_result.putFloat(i_r1_x, row, 0);
      bank_result.putFloat(i_r1_y, row, 0);
      bank_result.putFloat(i_r1_z, row, 0);
      bank_result.putByte(i_r2_found, row, 0);
      bank_result.putFloat(i_r2_x, row, 0);
      bank_result.putFloat(i_r2_y, row, 0);
      bank_result.putFloat(i_r2_z, row, 0);
      bank_result.putByte(i_r3_found, row, 0);
      bank_result.putFloat(i_r3_x, row, 0);
      bank_result.putFloat(i_r3_y, row, 0);
      bank_result.putFloat(i_r3_z, row, 0);
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
          if(bank_traj.getByte("detector", row_traj) == DetectorType::DC) { // only apply to DC
            switch(bank_traj.getInt("layer", row_traj)) {
              case 6: // region 1
                link_particle.r1_found = 1;
                link_particle.r1_x = bank_traj.getFloat("x", row_traj);
                link_particle.r1_y = bank_traj.getFloat("y", row_traj);
                link_particle.r1_z = bank_traj.getFloat("z", row_traj);
                break;
              case 18: // region 2
                link_particle.r2_found = 1;
                link_particle.r2_x = bank_traj.getFloat("x", row_traj);
                link_particle.r2_y = bank_traj.getFloat("y", row_traj);
                link_particle.r2_z = bank_traj.getFloat("z", row_traj);
                // Determine Sector from the center of the DC
                link_particle.sector = GetSector(link_particle.r2_x, link_particle.r2_y, link_particle.r2_z);
                break;
              case 36: // region 3
                link_particle.r3_found = 1;
                link_particle.r3_x = bank_traj.getFloat("x", row_traj);
                link_particle.r3_y = bank_traj.getFloat("y", row_traj);
                link_particle.r3_z = bank_traj.getFloat("z", row_traj);
                break;
            }
          }
        }
      }
      // fill output bank
      bank_result.putInt(i_sector,    row_particle, link_particle.sector);
      bank_result.putByte(i_r1_found, row_particle, link_particle.r1_found);
      bank_result.putFloat(i_r1_x,    row_particle, link_particle.r1_x);
      bank_result.putFloat(i_r1_y,    row_particle, link_particle.r1_y);
      bank_result.putFloat(i_r1_z,    row_particle, link_particle.r1_z);
      bank_result.putByte(i_r2_found, row_particle, link_particle.r2_found);
      bank_result.putFloat(i_r2_x,    row_particle, link_particle.r2_x);
      bank_result.putFloat(i_r2_y,    row_particle, link_particle.r2_y);
      bank_result.putFloat(i_r2_z,    row_particle, link_particle.r2_z);
      bank_result.putByte(i_r3_found, row_particle, link_particle.r3_found);
      bank_result.putFloat(i_r3_x,    row_particle, link_particle.r3_x);
      bank_result.putFloat(i_r3_y,    row_particle, link_particle.r3_y);
      bank_result.putFloat(i_r3_z,    row_particle, link_particle.r3_z);
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
