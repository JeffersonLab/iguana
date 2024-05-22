#include "Validator.h"

#include <TProfile.h>
#include <TStyle.h>

namespace iguana::clas12 {

  REGISTER_IGUANA_VALIDATOR(FiducialFilterValidator);

  void FiducialFilterValidator::Start(hipo::banklist& banks)
  {
    // define the algorithm sequence
    m_algo_seq = std::make_unique<AlgorithmSequence>();
    m_algo_seq->Add("clas12::EventBuilderFilter");
    m_algo_seq->Add("clas12::FiducialFilter");
    m_algo_seq->SetOption<std::vector<int>>("clas12::EventBuilderFilter", "pids", u_pdg_list);
    m_algo_seq->Start(banks);

    // get bank indices
    b_particle = GetBankIndex(banks, "REC::Particle");
    b_traj = GetBankIndex(banks, "REC::Traj"); 
      
    // set an output file
    auto output_dir = GetOutputDirectory();
    if(output_dir) {
      m_output_file_basename = output_dir.value() + "/fiducial";
      m_output_file          = new TFile(m_output_file_basename + ".root", "RECREATE");
    }

    // define plots
    gStyle->SetOptStat(0);
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


  void FiducialFilterValidator::Run(hipo::banklist& banks) const
  {
    // get the momenta before
    auto& particle_bank = GetBank(banks, b_particle, "REC::Particle");
    auto& trajBank      = GetBank(banks, b_traj, "REC::Traj");
    // get a pindex'd map of the REC::Traj data
    auto traj_map = GetTrajMap(trajBank);
    
    for(auto const& row : particle_bank.getRowList()){
        auto pid = particle_bank.getInt("pid", row);
        if(pid!=11&&pid!=211&&pid!=-211&&pid!=2212) continue;
        if (traj_map.find(row) == traj_map.end()) {
            continue;
        }
        auto traj_row = traj_map.at(row); 
        u_DC1_before.at(pid)->Fill(traj_row.x1,traj_row.y1);
        u_DC2_before.at(pid)->Fill(traj_row.x2,traj_row.y2);
        u_DC3_before.at(pid)->Fill(traj_row.x3,traj_row.y3);
    }

    // run the fiducial cuts
    m_algo_seq->Run(banks);

    // fill the plots
    for(auto const& row : particle_bank.getRowList()) {

        auto pid = particle_bank.getInt("pid", row);
        if(pid!=11&&pid!=211&&pid!=-211&&pid!=2212) continue;
        if (traj_map.find(row) == traj_map.end()) {
            continue;
        }
        auto traj_row = traj_map.at(row); 
        u_DC1_after.at(pid)->Fill(traj_row.x1,traj_row.y1);
        u_DC2_after.at(pid)->Fill(traj_row.x2,traj_row.y2);
        u_DC3_after.at(pid)->Fill(traj_row.x3,traj_row.y3);
    }
  }


  void FiducialFilterValidator::Stop()
  {
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


  int FiducialFilterValidator::determineSectorDC(float x, float y, float z) const
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
    
  std::map<int, traj_row_data> FiducialFilterValidator::GetTrajMap(hipo::bank const &bank) const
  {
      std::map<int, traj_row_data> traj_map;
      
      for(int row = 0; row < bank.getRows(); row++){
          auto pindex = bank.getInt("pindex",row);
          auto x      = bank.getFloat("x",row);
          auto y      = bank.getFloat("y",row);
          auto z      = bank.getFloat("z",row);
          auto layer  = bank.getInt("layer",row);
          
          // Ensure an entry exists in the map for the given pindex
          if (traj_map.find(pindex) == traj_map.end()) {
            traj_map[pindex] = traj_row_data();
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
}
