#include "Algorithm.h"
#include "Pass1CutData.h"

namespace iguana::clas12 {

  REGISTER_IGUANA_ALGORITHM(FiducialFilter);

  void FiducialFilter::Start(hipo::banklist& banks)
  {
    ParseYAMLConfig();
    o_pass = GetOptionScalar<int>("pass");
    if(o_pass!=1){
      m_log->Warn("FiducialFilter only contains fiducial cuts for pass1...we will default to using those...");
      o_pass = 1;
    }
    o_pcal_electron_cut_level = ParseCutLevel(GetOptionScalar<std::string>("pcal_electron_cut_level"));
    o_pcal_photon_cut_level   = ParseCutLevel(GetOptionScalar<std::string>("pcal_photon_cut_level"));
    o_enable_pcal_cuts        = GetOptionScalar<int>("enable_pcal_cuts") == 1;
    o_enable_dc_cuts          = GetOptionScalar<int>("enable_dc_cuts") == 1;

    b_particle = GetBankIndex(banks, "REC::Particle");
    b_config   = GetBankIndex(banks, "RUN::config");
    if(o_pass == 1) {
      b_traj = GetBankIndex(banks, "REC::Particle::Traj");
      b_cal  = GetBankIndex(banks, "REC::Particle::Calorimeter");
    }
  }

  //////////////////////////////////////////////////////////////////////////////////

  void FiducialFilter::Run(hipo::banklist& banks) const {

    auto& particleBank = GetBank(banks, b_particle, "REC::Particle");
    auto& configBank   = GetBank(banks, b_config, "RUN::config");
    auto torus         = configBank.getFloat("torus", 0);

    ShowBank(particleBank, Logger::Header("INPUT PARTICLES"));

    if(o_pass == 1) {
      auto& trajBank = GetBank(banks, b_traj, "REC::Particle::Traj");
      auto& calBank  = GetBank(banks, b_cal, "REC::Particle::Calorimeter");
      if(auto num_rows{particleBank.getRows()}; num_rows != trajBank.getRows() || num_rows != calBank.getRows()) {
        m_log->Error("number of particle bank rows differs from 'REC::Particle::Traj' and/or 'REC::Particle::Calorimeter' rows; are you sure these input banks are being filled?");
        throw std::runtime_error("cannot proceed");
      }
      particleBank.getMutableRowList().filter([this, torus, &trajBank, &calBank](hipo::bank& bank, int row) {
          auto pid = bank.getInt("pid", row);
          if(row >= 0 && row < calBank.getRows() && row < trajBank.getRows()) {
            // if we don't have the required info, return false
            if(o_enable_pcal_cuts && calBank.getByte("pcal_found", row) == 0)
              return false;
            if(o_enable_dc_cuts && (trajBank.getByte("r1_found", row) == 0 || trajBank.getByte("r2_found", row) == 0 || trajBank.getByte("r3_found", row) == 0))
              return false;
            // call the action function
            return FilterRgaPass1(
                calBank.getInt("pcal_sector", row),
                calBank.getFloat("pcal_lv", row),
                calBank.getFloat("pcal_lw", row),
                trajBank.getInt("sector", row),
                trajBank.getFloat("r1_x", row),
                trajBank.getFloat("r1_y", row),
                trajBank.getFloat("r1_z", row),
                trajBank.getFloat("r2_x", row),
                trajBank.getFloat("r2_y", row),
                trajBank.getFloat("r2_z", row),
                trajBank.getFloat("r3_x", row),
                trajBank.getFloat("r3_y", row),
                trajBank.getFloat("r3_z", row),
                torus,
                pid);
          }
          else
            throw std::runtime_error(fmt::format("FiducialFilter filter encountered bad row number {}", row));
          });
    }

    ShowBank(particleBank, Logger::Header("OUTPUT PARTICLES"));
  }

  //////////////////////////////////////////////////////////////////////////////////

  void FiducialFilter::Stop()
  {
  }

  //////////////////////////////////////////////////////////////////////////////////

  FiducialFilter::CutLevel FiducialFilter::ParseCutLevel(std::string const& level) const
  {
    if(level == "loose")
      return loose;
    else if(level == "medium")
      return medium;
    else if(level == "tight")
      return tight;
    else
      throw std::runtime_error(std::string("unknown PCAL cut level ") + level);
  }

  //////////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////////

  bool FiducialFilter::FilterRgaPass1(
      int const pcal_sector,
      float const pcal_lv,
      float const pcal_lw,
      int const dc_sector,
      float const dc_r1_x,
      float const dc_r1_y,
      float const dc_r1_z,
      float const dc_r2_x,
      float const dc_r2_y,
      float const dc_r2_z,
      float const dc_r3_x,
      float const dc_r3_y,
      float const dc_r3_z,
      float const torus,
      int const pid) const
  {

    // reject if torus is not +/-1
    if(std::abs(torus)!=1) {
      m_log->Warn("torus={}...value must be either -1 or 1, otherwise fiducial cuts are not defined...filtering out all particles...",torus);
      return false;
    }
    // apply cuts
    bool result = true;
    switch(pid) {
      case 11: // electrons
      case -11: // positrons
        result &= FilterPcalHomogeneous(pcal_sector, pcal_lv, pcal_lw, torus, pid);
        result &= FilterDcXY(dc_sector, dc_r1_x, dc_r1_y, dc_r1_z, dc_r2_x, dc_r2_y, dc_r2_z, dc_r3_x, dc_r3_y, dc_r3_z, torus, pid);
        break;
      case 22: // photons
        result &= FilterPcalHomogeneous(pcal_sector, pcal_lv, pcal_lw, torus, pid);
      case 211: // pi+
      case -211: // pi-
      case 2212: // protons
        {
          if(torus<0) // inbending
            result &= FilterDcThetaPhi(dc_sector, dc_r1_x, dc_r1_y, dc_r1_z, dc_r2_x, dc_r2_y, dc_r2_z, dc_r3_x, dc_r3_y, dc_r3_z, torus, pid);
          else if(torus>0) // outbending
            result &= FilterDcXY(dc_sector, dc_r1_x, dc_r1_y, dc_r1_z, dc_r2_x, dc_r2_y, dc_r2_z, dc_r3_x, dc_r3_y, dc_r3_z, torus, pid);
          else
            result = false;
          break;
        }
      default:
        result = true; // cut not applied, do not filter
    }
    return result;
  }

  //////////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////////

  bool FiducialFilter::FilterPcalHomogeneous(
      int const sector,
      float const lv,
      float const lw,
      float const torus,
      int const pid) const
  {
    if(!o_enable_pcal_cuts)
      return true;
    // set cut level from PDG
    CutLevel cut_level;
    switch(pid) {
      case 11:
      case -11:
        cut_level = o_pcal_electron_cut_level;
        break;
      case 22:
        cut_level = o_pcal_photon_cut_level;
        break;
      default:
        m_log->Error("called FilterPcalHomogeneous with unknown PDG {}", pid);
        return false;
    }
    // NOTE: lv + lw is going from the side to the back end of the PCAL, lu is going from side to side
    // 1 scintillator bar is 4.5 cm wide. In the outer regions (back) double bars are used.
    // A cut is only applied on lv and lw
    double min_v = 0;
    double max_v = 0;
    double min_w = 0;
    double max_w = 0;
    for(int k = 0; k < 6; k++){
      if(sector-1 == k && torus < 0){ // inbending
        switch(cut_level) {
          case tight:
            min_v = fiducial_pass1::min_lv_tight_inb[k];
            max_v = fiducial_pass1::max_lv_tight_inb[k];
            min_w = fiducial_pass1::min_lw_tight_inb[k];
            max_w = fiducial_pass1::max_lw_tight_inb[k];
            break;
          case medium:
            min_v = fiducial_pass1::min_lv_med_inb[k];
            max_v = fiducial_pass1::max_lv_med_inb[k];
            min_w = fiducial_pass1::min_lw_med_inb[k];
            max_w = fiducial_pass1::max_lw_med_inb[k];
            break;
          case loose:
            min_v = fiducial_pass1::min_lv_loose_inb[k];
            max_v = fiducial_pass1::max_lv_loose_inb[k];
            min_w = fiducial_pass1::min_lw_loose_inb[k];
            max_w = fiducial_pass1::max_lw_loose_inb[k];
            break;
        }
      }
      else if(sector-1 == k && torus > 0){ // outbending
        switch(cut_level) {
          case tight:
            min_v = fiducial_pass1::min_lv_tight_out[k];
            max_v = fiducial_pass1::max_lv_tight_out[k];
            min_w = fiducial_pass1::min_lw_tight_out[k];
            max_w = fiducial_pass1::max_lw_tight_out[k];
            break;
          case medium:
            min_v = fiducial_pass1::min_lv_med_out[k];
            max_v = fiducial_pass1::max_lv_med_out[k];
            min_w = fiducial_pass1::min_lw_med_out[k];
            max_w = fiducial_pass1::max_lw_med_out[k];
            break;
          case loose:
            min_v = fiducial_pass1::min_lv_loose_out[k];
            max_v = fiducial_pass1::max_lv_loose_out[k];
            min_w = fiducial_pass1::min_lw_loose_out[k];
            max_w = fiducial_pass1::max_lw_loose_out[k];
            break;
        }
      }
    }
    return lv > min_v && lv < max_v && lw > min_w && lw < max_w;
  }

  //////////////////////////////////////////////////////////////////////////////////

  bool FiducialFilter::FilterDcXY(
      int const sector,
      float const r1_x,
      float const r1_y,
      float const r1_z,
      float const r2_x,
      float const r2_y,
      float const r2_z,
      float const r3_x,
      float const r3_y,
      float const r3_z,
      float const torus,
      int const pid) const
  {
    if(!o_enable_dc_cuts)
      return true;
    if(!(sector >= 1 && sector <= 6))
      return false;
    const auto& minparams = ((torus<0) ? fiducial_pass1::minparams_in_XY_pass1 : fiducial_pass1::minparams_out_XY_pass1);
    const auto& maxparams = ((torus<0) ? fiducial_pass1::maxparams_in_XY_pass1 : fiducial_pass1::maxparams_out_XY_pass1);
    double X=0;
    double Y=0;
    for(int region = 0 ; region < 3; region++){
      X=0;
      Y=0;
      switch(region){
        case 0:
          X = r1_x;
          Y = r1_y;
          break;
        case 1:
          X = r2_x;
          Y = r2_y;
          break;
        case 2:
          X = r3_x;
          Y = r3_y;
          break;
      }
      if(sector == 2)
      {
        const double X_new = X * std::cos(-60 * M_PI / 180) - Y * std::sin(-60 * M_PI / 180);
        Y = X * std::sin(-60 * M_PI / 180) + Y * std::cos(-60 * M_PI / 180);
        X = X_new;
      }
      if(sector == 3)
      {
        const double X_new = X * std::cos(-120 * M_PI / 180) - Y * std::sin(-120 * M_PI / 180);
        Y = X * std::sin(-120 * M_PI / 180) + Y * std::cos(-120 * M_PI / 180);
        X = X_new;
      }
      if(sector == 4)
      {
        const double X_new = X * std::cos(-180 * M_PI / 180) - Y * std::sin(-180 * M_PI / 180);
        Y = X * std::sin(-180 * M_PI / 180) + Y * std::cos(-180 * M_PI / 180);
        X = X_new;
      }
      if(sector == 5)
      {
        const double X_new = X * std::cos(120 * M_PI / 180) - Y * std::sin(120 * M_PI / 180);
        Y = X * std::sin(120 * M_PI / 180) + Y * std::cos(120 * M_PI / 180);
        X = X_new;
      }
      if(sector == 6)
      {
        const double X_new = X * std::cos(60 * M_PI / 180) - Y * std::sin(60 * M_PI / 180);
        Y = X * std::sin(60 * M_PI / 180) + Y * std::cos(60 * M_PI / 180);
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
      double calc_min = minparams[this_pid][sector - 1][region][0] + minparams[this_pid][sector - 1][region][1] * X;
      double calc_max = maxparams[this_pid][sector - 1][region][0] + maxparams[this_pid][sector - 1][region][1] * X;
      if(std::isnan(calc_min)||std::isnan(calc_max)) return false;
      if((Y<calc_min) || (Y>calc_max)) {  return false;}
    }
    return true;
  }

  //////////////////////////////////////////////////////////////////////////////////

  bool FiducialFilter::FilterDcThetaPhi(
      int const sector,
      float const r1_x,
      float const r1_y,
      float const r1_z,
      float const r2_x,
      float const r2_y,
      float const r2_z,
      float const r3_x,
      float const r3_y,
      float const r3_z,
      float const torus,
      int const pid) const
  {
    if(!o_enable_dc_cuts)
      return true;
    if(!(sector >= 1 && sector <= 6))
      return false;
    const auto& minparams = ((torus<0) ? fiducial_pass1::minparams_in_theta_phi_pass1 : fiducial_pass1::minparams_out_theta_phi_pass1);
    const auto& maxparams = ((torus<0) ? fiducial_pass1::maxparams_in_theta_phi_pass1 : fiducial_pass1::maxparams_out_theta_phi_pass1);
    double theta_DCr = 5000;
    double phi_DCr_raw = 5000;
    double x=0;
    double y=0;
    double z=0;
    for(int region = 0; region<3; region++){
      x=0;y=0;z=0;
      switch(region){
        case 0:
          x=r1_x;
          y=r1_y;
          z=r1_z;
          break;
        case 1:
          x=r2_x;
          y=r2_y;
          z=r2_z;
          break;
        case 2:
          x=r3_x;
          y=r3_y;
          z=r3_z;
          break;
      }
      theta_DCr = 180 / M_PI * acos(z / sqrt(pow(x,2) + pow(y,2) + pow(z,2)));
      phi_DCr_raw = 180 / M_PI * atan2(y / sqrt(pow(x,2) + pow(y,2) + pow(z,2)),
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
      double calc_phi_min = minparams[this_pid][sector - 1][region][0] + minparams[this_pid][sector - 1][region][1] * std::log(theta_DCr)
        + minparams[this_pid][sector - 1][region][2] * theta_DCr + minparams[this_pid][sector - 1][region][3] * theta_DCr * theta_DCr;
      double calc_phi_max = maxparams[this_pid][sector - 1][region][0] + maxparams[this_pid][sector - 1][region][1] * std::log(theta_DCr)
        + maxparams[this_pid][sector - 1][region][2] * theta_DCr + maxparams[this_pid][sector - 1][region][3] * theta_DCr * theta_DCr;
      if(std::isnan(calc_phi_min)||std::isnan(calc_phi_max)) return false;
      if((phi_DCr < calc_phi_min) || (phi_DCr > calc_phi_max)) return false;
    }
    return true;
  }

  //////////////////////////////////////////////////////////////////////////////////

}
