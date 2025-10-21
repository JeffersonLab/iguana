#include "Validator.h"

#include <TProfile.h>

namespace iguana::clas12::rga {

  REGISTER_IGUANA_VALIDATOR(FiducialFilterPass1Validator);

  void FiducialFilterPass1Validator::Start(hipo::banklist& banks)
  {
    // set algorithm options
    m_algo_eb.SetOption<std::vector<int>>("pids", u_pdg_list);

    // start algorithms
    m_algo_eb.Start(banks);
    m_algo_traj.Start(banks);
    m_algo_cal.Start(banks);
    m_algo_fidu.Start(banks);

    // get bank indices
    b_particle = GetBankIndex(banks, "REC::Particle");
    b_traj     = GetBankIndex(banks, "REC::Particle::Traj");
    b_cal      = GetBankIndex(banks, "REC::Particle::Calorimeter");
      
    // set an output file
    auto output_dir = GetOutputDirectory();
    if(output_dir) {
      m_output_file_basename = output_dir.value() + "/fiducial";
      m_output_file          = new TFile(m_output_file_basename + ".root", "RECREATE");
    }

    // define plots
    for(auto const& pdg : u_pdg_list) {
      TString particle_name  = particle::name.at(particle::PDG(pdg));
      TString particle_title = particle::title.at(particle::PDG(pdg));
        
      TH2D *DC1_before = new TH2D(
          "DC1_before_"+particle_name,
          particle_title + " DC1 w/o fiducial cuts; x [cm]; y [cm]",
          100, DC1xleft, DC1xright,
          100, DC1ybottom, DC1ytop
      );
      TH2D *DC1_after = new TH2D(
          "DC1_after_"+particle_name,
          particle_title + " DC1 w fiducial cuts; x [cm]; y [cm]",
          100, DC1xleft, DC1xright,
          100, DC1ybottom, DC1ytop
      );
        
      TH2D *DC2_before = new TH2D(
            "DC2_before_" + particle_name,
            particle_title + " DC2 w/o fiducial cuts; x [cm]; y [cm]",
            100, DC2xleft, DC2xright,
            100, DC2ybottom, DC2ytop
      );
      TH2D *DC2_after = new TH2D(
            "DC2_after_" + particle_name,
            particle_title + " DC2 w fiducial cuts; x [cm]; y [cm]",
            100, DC2xleft, DC2xright,
            100, DC2ybottom, DC2ytop
      );

      TH2D *DC3_before = new TH2D(
            "DC3_before_" + particle_name,
            particle_title + " DC3 w/o fiducial cuts; x [cm]; y [cm]",
            100, DC3xleft, DC3xright,
            100, DC3ybottom, DC3ytop
      );
      TH2D *DC3_after = new TH2D(
            "DC3_after_" + particle_name,
            particle_title + " DC3 w fiducial cuts; x [cm]; y [cm]",
            100, DC3xleft, DC3xright,
            100, DC3ybottom, DC3ytop
      );
      
      u_DC1_before.insert({pdg, DC1_before});
      u_DC1_after.insert({pdg, DC1_after});
      u_DC2_before.insert({pdg, DC2_before});
      u_DC2_after.insert({pdg, DC2_after});
      u_DC3_before.insert({pdg, DC3_before});
      u_DC3_after.insert({pdg, DC3_after});
    }
  }


  bool FiducialFilterPass1Validator::Run(hipo::banklist& banks) const
  {
    auto& particle_bank = GetBank(banks, b_particle, "REC::Particle");
    auto& traj_bank     = GetBank(banks, b_traj, "REC::Particle::Traj");

    // run the EB filter and TrajLinker
    m_algo_eb.Run(banks);
    m_algo_traj.Run(banks);
    m_algo_cal.Run(banks);

    // fill "before" histograms
    for(auto const& row : particle_bank.getRowList()){
      auto pid = particle_bank.getInt("pid", row);
      if(pid!=11&&pid!=211&&pid!=-211&&pid!=2212) continue;
      if(traj_bank.getByte("r1_found", row) == 1)
        u_DC1_before.at(pid)->Fill(traj_bank.getFloat("r1_x", row), traj_bank.getFloat("r1_y", row));
      if(traj_bank.getByte("r2_found", row) == 1)
        u_DC2_before.at(pid)->Fill(traj_bank.getFloat("r2_x", row), traj_bank.getFloat("r2_y", row));
      if(traj_bank.getByte("r3_found", row) == 1)
        u_DC3_before.at(pid)->Fill(traj_bank.getFloat("r3_x", row), traj_bank.getFloat("r3_y", row));
    }

    // apply the fiducial cuts
    m_algo_fidu.Run(banks);

    // fill "after" histograms (`particle_bank` is now filtered)
    for(auto const& row : particle_bank.getRowList()) {
      auto pid = particle_bank.getInt("pid", row);
      if(pid!=11&&pid!=211&&pid!=-211&&pid!=2212) continue;
      if(traj_bank.getByte("r1_found", row) == 1)
        u_DC1_after.at(pid)->Fill(traj_bank.getFloat("r1_x", row), traj_bank.getFloat("r1_y", row));
      if(traj_bank.getByte("r2_found", row) == 1)
        u_DC2_after.at(pid)->Fill(traj_bank.getFloat("r2_x", row), traj_bank.getFloat("r2_y", row));
      if(traj_bank.getByte("r3_found", row) == 1)
        u_DC3_after.at(pid)->Fill(traj_bank.getFloat("r3_x", row), traj_bank.getFloat("r3_y", row));
    }
    return true;
  }


  void FiducialFilterPass1Validator::Stop()
  {
    m_algo_eb.Stop();
    m_algo_traj.Stop();
    m_algo_cal.Stop();
    m_algo_fidu.Stop();
    if(GetOutputDirectory()) {
      int n_cols = 2;
      int n_rows = 2;
      for(int r=0;r<3;r++){
          TString canv_name = Form("beforecanv%d", r);
          auto canv         = new TCanvas(canv_name, canv_name, n_cols * 800, n_rows * 600);
          canv->Divide(n_cols, n_rows);
          int pad_num = 0;
          for(auto const& pdg : u_pdg_list){
              auto pad = canv->GetPad(++pad_num);
              pad->cd();
              pad->SetGrid(1, 1);
              pad->SetLogz();
              pad->SetLeftMargin(0.12);
              pad->SetRightMargin(0.12);
              pad->SetBottomMargin(0.12);
              if(r==0){
                  u_DC1_before.at(pdg)->Draw("colz");
              }else if(r==1){
                  u_DC2_before.at(pdg)->Draw("colz");
              }else{
                  u_DC3_before.at(pdg)->Draw("colz");
              }
          }
          canv->SaveAs(m_output_file_basename + "_before_DC" + std::to_string(r+1) + ".png");
      }
        
      for(int r=0;r<3;r++){
          TString canv_name = Form("aftercanv%d", r);
          auto canv         = new TCanvas(canv_name, canv_name, n_cols * 800, n_rows * 600);
          canv->Divide(n_cols, n_rows);
          int pad_num = 0;
          for(auto const& pdg : u_pdg_list){
              auto pad = canv->GetPad(++pad_num);
              pad->cd();
              pad->SetGrid(1, 1);
              pad->SetLogz();
              pad->SetLeftMargin(0.12);
              pad->SetRightMargin(0.12);
              pad->SetBottomMargin(0.12);
              if(r==0){
                  u_DC1_after.at(pdg)->Draw("colz");
              }else if(r==1){
                  u_DC2_after.at(pdg)->Draw("colz");
              }else{
                  u_DC3_after.at(pdg)->Draw("colz");
              }
          }
          canv->SaveAs(m_output_file_basename + "_after_DC" + std::to_string(r+1) + ".png");
      } 
      m_output_file->Write();
      m_log->Info("Wrote output file {}", m_output_file->GetName());
      m_output_file->Close();
    }
  }
}
