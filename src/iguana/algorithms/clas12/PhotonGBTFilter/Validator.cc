#include "Validator.h"
#include "Algorithm.h"
namespace iguana::clas12 {

  REGISTER_IGUANA_VALIDATOR(PhotonGBTFilterValidator);

  void PhotonGBTFilterValidator::Start(hipo::banklist& banks)
  {
    // define the algorithm sequence
    m_algo_seq = std::make_unique<AlgorithmSequence>();
    m_algo_seq->Add("clas12::PhotonGBTFilter"); 
    m_algo_seq->Add("clas12::EventBuilderFilter");
    m_algo_seq->SetOption<std::vector<int>>("clas12::EventBuilderFilter", "pids", u_pdg_list);
    m_algo_seq->Start(banks);

    // get bank indices
    b_particle = GetBankIndex(banks, "REC::Particle");

    // set an output file
    auto output_dir = GetOutputDirectory();
    if(output_dir) {
      m_output_file_basename = output_dir.value() + "/photon_gbt";
      m_output_file          = new TFile(m_output_file_basename + ".root", "RECREATE");
    }
      
    InitializeHistograms();
  }

  void PhotonGBTFilterValidator::Run(hipo::banklist& banks) const
  {
    // get the particle bank
    auto& particle_bank = GetBank(banks, b_particle, "REC::Particle");
    
    std::vector<ROOT::Math::PxPyPzEVector> photons;
    for(auto const& row : particle_bank.getRowList()) {
        auto pid = particle_bank.getInt("pid",row);
        if(pid!=22) continue;
        float px = particle_bank.getFloat("px",row);
        float py = particle_bank.getFloat("py",row);
        float pz = particle_bank.getFloat("pz",row);
        float E  = std::hypot(px,py,pz);
        ROOT::Math::PxPyPzEVector phot(px,py,pz,E);
        if(m_algo_seq->Get<PhotonGBTFilter>("clas12::PhotonGBTFilter")->ForwardDetectorFilter(phot.Theta()))
          photons.push_back(phot);
    }

    // run the photon filter
    m_algo_seq->Run(banks);
      
    std::vector<ROOT::Math::PxPyPzEVector> filtered_photons;
    for(auto const& row : particle_bank.getRowList()) {
        auto pid = particle_bank.getInt("pid",row);
        if(pid!=22) continue;
        float px = particle_bank.getFloat("px",row);
        float py = particle_bank.getFloat("py",row);
        float pz = particle_bank.getFloat("pz",row);
        float E  = std::hypot(px,py,pz);
        ROOT::Math::PxPyPzEVector phot(px,py,pz,E);
        filtered_photons.push_back(phot);
    }
      
    // fill the plots
    FillHistograms(photons, 0);
    FillHistograms(filtered_photons, 1);

  }
    
  void PhotonGBTFilterValidator::InitializeHistograms() {
    std::vector<std::pair<int, TString>> histInfos = {
        {0, "before"},
        {1, "after"}
    };

    for (const auto& [idx, label] : histInfos) {
        h_Mgg[idx] = new TH1F(Form("h_Mgg_%s", label.Data()), ";M_{#gamma#gamma} [GeV]", 100, 0.02, 0.5);
        h_P[idx] = new TH1F(Form("h_P_%s", label.Data()), ";P(#gamma) [GeV]", 100, 0, 2);
        h_Th[idx] = new TH1F(Form("h_Th_%s", label.Data()), ";#theta(#gamma) [deg]", 100, 0, 36);
        h_Phi[idx] = new TH1F(Form("h_Phi_%s", label.Data()), ";#phi(#gamma) [deg]", 100, -180, 180);
        
        int color = (idx == 0) ? kBlack : kRed;

        ConfigureHistogram(h_Mgg[idx], color);
        ConfigureHistogram(h_P[idx], color);
        ConfigureHistogram(h_Th[idx], color);
        ConfigureHistogram(h_Phi[idx], color);
    }
  }

  void PhotonGBTFilterValidator::ConfigureHistogram(TH1F* hist, int color) {
    hist->SetLineColor(color);
    hist->SetLineWidth(2);
    hist->GetXaxis()->SetTitleSize(0.06);
    hist->GetYaxis()->SetTitleSize(0.06);
  }
    
  void PhotonGBTFilterValidator::FillHistograms(const std::vector<ROOT::Math::PxPyPzEVector>& photons, int idx) const
  {
    for (const auto& photon : photons) {
        h_P.at(idx)->Fill(photon.P());
        h_Th.at(idx)->Fill(photon.Theta() * 180.0 / M_PI);
        h_Phi.at(idx)->Fill(photon.Phi() * 180.0 / M_PI);
    }

    for (size_t i = 0; i < photons.size(); ++i) {
        for (size_t j = i + 1; j < photons.size(); ++j) {
            auto diphoton = photons[i] + photons[j];
            h_Mgg.at(idx)->Fill(diphoton.M());
        }
    }
  }
    
  void PhotonGBTFilterValidator::Stop()
  {
      if(GetOutputDirectory()){
          int n_rows = 2;
          int n_cols = 2;
          auto canv = new TCanvas("c","c",n_cols*800,n_rows*800);
          canv->Divide(n_cols,n_rows);
          for(int pad_num = 0; pad_num < 4; pad_num++){
              auto pad = canv->GetPad(pad_num+1);
              pad->cd();
              pad->SetGrid(1, 1);
              pad->SetLogz();
              pad->SetLeftMargin(0.12);
              pad->SetRightMargin(0.12);
              pad->SetBottomMargin(0.12);
              switch(pad_num){
                  case 0:
                      h_Mgg[0]->Draw("hist");
                      h_Mgg[1]->Draw("hist same");
                      break;
                  case 1:
                      h_P[0]->Draw("hist");
                      h_P[1]->Draw("hist same");
                      break;
                  case 2:
                      h_Th[0]->Draw("hist");
                      h_Th[1]->Draw("hist same");
                      break;
                  case 3:
                      h_Phi[0]->Draw("hist");
                      h_Phi[1]->Draw("hist same");
                      break;
              }
          }
        canv->SaveAs(m_output_file_basename + "_plot.png");
        m_output_file->Write();
        m_log->Info("Wrote output file {}", m_output_file->GetName());
        m_output_file->Close();
      }
  }

}
