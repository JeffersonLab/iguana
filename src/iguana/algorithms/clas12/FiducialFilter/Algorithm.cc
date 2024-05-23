#include "Algorithm.h"

namespace iguana::clas12 {

  REGISTER_IGUANA_ALGORITHM(FiducialFilter);

  void FiducialFilter::Start(hipo::banklist& banks)
  {

    // Read YAML config file
    ParseYAMLConfig();
    b_particle = GetBankIndex(banks, "REC::Particle");
    b_traj     = GetBankIndex(banks, "REC::Traj");
    b_config   = GetBankIndex(banks, "RUN::config");
      
    o_pass      = GetCachedOption<int>("pass").value_or(1);
    if(o_pass!=1){
        m_log->Warn("FiducialFilter only contains fiducial cuts for pass1...we will default to using those...");
    }
      
  }

void FiducialFilter::Run(hipo::banklist& banks) const {
    // get the banks
    auto& particleBank = GetBank(banks, b_particle, "REC::Particle");
    auto& trajBank     = GetBank(banks, b_traj, "REC::Traj");
    auto& configBank   = GetBank(banks, b_config, "RUN::config");
    auto torus  = (int)configBank.getFloat("torus", 0);    
    
    // dump the bank
    ShowBank(particleBank, Logger::Header("INPUT PARTICLES"));

    // get a pindex'd map of the REC::Traj data
    auto traj_map = GetTrajMap(trajBank);
    
    // filter the input bank for requested PDG code(s)
    particleBank.getMutableRowList().filter([this, &traj_map, torus](hipo::bank& bank, int row) {
        // Check if this particle has a REC::Traj component
        if (traj_map.find(row) == traj_map.end()) {
            return false; // particle not in REC::traj
        }
        auto traj_row = traj_map.at(row); 
        auto pid = bank.getInt("pid", row);
        auto accept  = Filter(traj_row, torus, pid);
        return accept;
    });

    // dump the modified bank
    ShowBank(particleBank, Logger::Header("OUTPUT PARTICLES"));
}

  bool FiducialFilter::Filter(FiducialFilter::traj_row_data const traj_row, int const torus, int const pid) const
  {

    if(pid==11) return DC_fiducial_cut_XY_pass1(traj_row, torus, pid);
    else if(pid==211 || pid==-211 || pid==2212){
        if(torus==-1) return DC_fiducial_cut_theta_phi_pass1(traj_row, torus, pid);
        else if(torus==1) return DC_fiducial_cut_XY_pass1(traj_row, torus, pid);
        else return true;
    }
    return true;
  }

    

  bool FiducialFilter::DC_fiducial_cut_XY_pass1(FiducialFilter::traj_row_data const traj_row, int const torus, int const pid) const
  {
    
      const auto minparams = ((torus==-1) ? minparams_in_XY_pass1 : minparams_out_XY_pass1);
      const auto maxparams = ((torus==-1) ? maxparams_in_XY_pass1 : maxparams_out_XY_pass1);
      double X=0;
      double Y=0;
      for(int r = 0 ; r < 3; r++){
        X=0;
        Y=0;
        switch(r){
        case 0:
          X = traj_row.x1;
          Y = traj_row.y1;
          break;
        case 1:
          X = traj_row.x2;
          Y = traj_row.y2;
          break;
        case 2:
          X = traj_row.x3;
          Y = traj_row.y3;
          break;
        }

        int sector = traj_row.sector;

        if(sector == 2)
          {
        const double X_new = X * std::cos(-60 * PI / 180) - Y * std::sin(-60 * PI / 180);
        Y = X * std::sin(-60 * PI / 180) + Y * std::cos(-60 * PI / 180);
        X = X_new;
          }

        if(sector == 3)
          {
        const double X_new = X * std::cos(-120 * PI / 180) - Y * std::sin(-120 * PI / 180);
        Y = X * std::sin(-120 * PI / 180) + Y * std::cos(-120 * PI / 180);
        X = X_new;
          }

        if(sector == 4)
          {
        const double X_new = X * std::cos(-180 * PI / 180) - Y * std::sin(-180 * PI / 180);
        Y = X * std::sin(-180 * PI / 180) + Y * std::cos(-180 * PI / 180);
        X = X_new;
          }

        if(sector == 5)
          {
        const double X_new = X * std::cos(120 * PI / 180) - Y * std::sin(120 * PI / 180);
        Y = X * std::sin(120 * PI / 180) + Y * std::cos(120 * PI / 180);
        X = X_new;
          }

        if(sector == 6)
          {
        const double X_new = X * std::cos(60 * PI / 180) - Y * std::sin(60 * PI / 180);
        Y = X * std::sin(60 * PI / 180) + Y * std::cos(60 * PI / 180);
        X = X_new;
          }


        int this_pid = 0;

        switch (pid)
          {
          case 11: 
        this_pid = 0; 
        break;
          case 2212: 
        this_pid = 1; 
        break;
          case 211: 
        this_pid = 2; 
        break;
          case -211: 
        this_pid = 3; 
        break;
          case 321: 
        this_pid = 4; 
        break;
          case -321: 
        this_pid = 5; 
        break;
          default: 
        return false; 
        break;
          }
        double calc_min = minparams[this_pid][sector - 1][r][0] + minparams[this_pid][sector - 1][r][1] * X;
        double calc_max = maxparams[this_pid][sector - 1][r][0] + maxparams[this_pid][sector - 1][r][1] * X;
        if(std::isnan(calc_min)||std::isnan(calc_max)) return false;
        if((Y<calc_min) || (Y>calc_max)) {  return false;}
      }
      return true;
    }    
    
  bool FiducialFilter::DC_fiducial_cut_theta_phi_pass1(FiducialFilter::traj_row_data const traj_row, int const torus, int const pid) const{
      
      const auto minparams = ((torus==-1) ? minparams_in_theta_phi_pass1 : minparams_out_theta_phi_pass1);
      const auto maxparams = ((torus==-1) ? maxparams_in_theta_phi_pass1 : maxparams_out_theta_phi_pass1);
      double theta_DCr = 5000;
      double phi_DCr_raw = 5000;
      double x=0;
      double y=0;
      double z=0;
      for(int r = 0; r<3; r++){  
        x=0;y=0;z=0;
        switch(r){
        case 0:
          x=traj_row.x1;
          y=traj_row.y1;
          z=traj_row.z1;
          break;
        case 1:
          x=traj_row.x2;
          y=traj_row.y2;
          z=traj_row.z2;
          break;
        case 2:
          x=traj_row.x3;
          y=traj_row.y3;
          z=traj_row.z3;
          break;
        }
        int sector = traj_row.sector;

        theta_DCr = 180 / PI * acos(z / sqrt(pow(x,2) + pow(y,2) + pow(z,2)));
        phi_DCr_raw = 180 / PI * atan2(y / sqrt(pow(x,2) + pow(y,2) + pow(z,2)), 
                       x /sqrt(pow(x,2) + pow(y,2) + pow(z,2)));

        double phi_DCr = 5000;

        if (sector == 1) phi_DCr = phi_DCr_raw;
        if (sector == 2) phi_DCr = phi_DCr_raw - 60;
        if (sector == 3) phi_DCr = phi_DCr_raw - 120;
        if (sector == 4 && phi_DCr_raw > 0) phi_DCr = phi_DCr_raw - 180;
        if (sector == 4 && phi_DCr_raw < 0) phi_DCr = phi_DCr_raw + 180;
        if (sector == 5) phi_DCr = phi_DCr_raw + 120;
        if (sector == 6) phi_DCr = phi_DCr_raw + 60;

        int this_pid = 0;

        switch (pid)
          {
          case 11: this_pid = 0; break;
          case 2212: this_pid = 1; break;
          case 211: this_pid = 2; break;
          case -211: this_pid = 3; break;
          case 321: this_pid = 4; break;
          case -321: this_pid = 5; break;
          default: return false; break;
          }



        double calc_phi_min = minparams[this_pid][sector - 1][r][0] + minparams[this_pid][sector - 1][r][1] * std::log(theta_DCr) 
          + minparams[this_pid][sector - 1][r][2] * theta_DCr + minparams[this_pid][sector - 1][r][3] * theta_DCr * theta_DCr;

        double calc_phi_max = maxparams[this_pid][sector - 1][r][0] + maxparams[this_pid][sector - 1][r][1] * std::log(theta_DCr)
          + maxparams[this_pid][sector - 1][r][2] * theta_DCr + maxparams[this_pid][sector - 1][r][3] * theta_DCr * theta_DCr;

        if(std::isnan(calc_phi_min)||std::isnan(calc_phi_max)) return false;
        if((phi_DCr < calc_phi_min) || (phi_DCr > calc_phi_max)) return false;
      }
      return true;
  }
  std::map<int, FiducialFilter::traj_row_data> FiducialFilter::GetTrajMap(hipo::bank const &bank) const
  {
      std::map<int, FiducialFilter::traj_row_data> traj_map;
      
      for(int row = 0; row < bank.getRows(); row++){
          auto pindex = bank.getInt("pindex",row);
          auto x      = bank.getFloat("x",row);
          auto y      = bank.getFloat("y",row);
          auto z      = bank.getFloat("z",row);
          auto layer  = bank.getInt("layer",row);
          
          // Ensure an entry exists in the map for the given pindex
          if (traj_map.find(pindex) == traj_map.end()) {
            traj_map[pindex] = FiducialFilter::traj_row_data();
          }
          
          switch(layer){
              case 6: // first DC
                traj_map[pindex].x1 = x;
                traj_map[pindex].y1 = y;
                traj_map[pindex].z1 = z;
                break;
              case 18: // second DC
                traj_map[pindex].x2 = x;
                traj_map[pindex].y2 = y;
                traj_map[pindex].z2 = z;
                // Determine Sector from the center of the DC
                traj_map[pindex].sector = determineSectorDC(traj_map[pindex].x2,traj_map[pindex].y2,traj_map[pindex].z2);
                break;
              case 36: // third DC
                traj_map[pindex].x3 = x;
                traj_map[pindex].y3 = y;
                traj_map[pindex].z3 = z;
                break;
          }
      }
      
      
      
      return traj_map;      
  }
      
  int FiducialFilter::determineSectorDC(float x, float y, float z) const
  {
      float phi = 180 / PI * atan2(y / sqrt(pow(x,2) + pow(y,2) + pow(z,2)),
                       x /sqrt(pow(x,2) + pow(y,2) + pow(z,2)));
      if(phi<30 && phi>=-30){return 1;}
      else if(phi<90 && phi>=30){return 2;}
      else if(phi<150 && phi>=90){return 3;}
      else if(phi>=150 || phi<-150){return 4;}
      else if(phi<-90 && phi>=-150){return 5;}
      else if(phi<-30 && phi>-90){return 6;}

      return 0;

  }
    
  void FiducialFilter::Stop()
  {
  }

}
