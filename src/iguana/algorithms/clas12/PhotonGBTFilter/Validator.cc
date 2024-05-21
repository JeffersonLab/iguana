#include "Validator.h"
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
      
    // Create the histograms for "before"
    h_Mgg[0] = new TH1F("h_Mgg_before", ";M_{#gamma#gamma} [GeV]", 100, 0, 0.5);
    h_P[0] = new TH1F("h_P_before", ";P(#gamma) [GeV]", 100, 0, 2);
    h_Th[0] = new TH1F("h_Th_before", ";#theta(#gamma) [deg]", 100, 0, 50);
    h_Phi[0] = new TH1F("h_Phi_before", ";#phi(#gamma) [deg]", 100, -180, 180);

    // Set properties for "before" histograms
    h_Mgg[0]->SetLineColor(kBlack);
    h_Mgg[0]->SetLineWidth(2);
    h_Mgg[0]->GetXaxis()->SetTitleSize(0.06);
    h_Mgg[0]->GetYaxis()->SetTitleSize(0.06);

    h_P[0]->SetLineColor(kBlack);
    h_P[0]->SetLineWidth(2);
    h_P[0]->GetXaxis()->SetTitleSize(0.06);
    h_P[0]->GetYaxis()->SetTitleSize(0.06);

    h_Th[0]->SetLineColor(kBlack);
    h_Th[0]->SetLineWidth(2);
    h_Th[0]->GetXaxis()->SetTitleSize(0.06);
    h_Th[0]->GetYaxis()->SetTitleSize(0.06);

    h_Phi[0]->SetLineColor(kBlack);
    h_Phi[0]->SetLineWidth(2);
    h_Phi[0]->GetXaxis()->SetTitleSize(0.06);
    h_Phi[0]->GetYaxis()->SetTitleSize(0.06);

    // Create the histograms for "after"
    h_Mgg[1] = new TH1F("h_Mgg_after", ";M_{#gamma#gamma} [GeV]", 100, 0, 0.5);
    h_P[1] = new TH1F("h_P_after", ";P(#gamma) [GeV]", 100, 0, 2);
    h_Th[1] = new TH1F("h_Th_after", ";#theta(#gamma) [deg]", 100, 0, 50);
    h_Phi[1] = new TH1F("h_Phi_after", ";#phi(#gamma) [deg]", 100, -180, 180);

    // Set properties for "after" histograms
    h_Mgg[1]->SetLineColor(kRed);
    h_Mgg[1]->SetLineWidth(2);
    h_Mgg[1]->GetXaxis()->SetTitleSize(0.06);
    h_Mgg[1]->GetYaxis()->SetTitleSize(0.06);

    h_P[1]->SetLineColor(kRed);
    h_P[1]->SetLineWidth(2);
    h_P[1]->GetXaxis()->SetTitleSize(0.06);
    h_P[1]->GetYaxis()->SetTitleSize(0.06);

    h_Th[1]->SetLineColor(kRed);
    h_Th[1]->SetLineWidth(2);
    h_Th[1]->GetXaxis()->SetTitleSize(0.06);
    h_Th[1]->GetYaxis()->SetTitleSize(0.06);

    h_Phi[1]->SetLineColor(kRed);
    h_Phi[1]->SetLineWidth(2);
    h_Phi[1]->GetXaxis()->SetTitleSize(0.06);
    h_Phi[1]->GetYaxis()->SetTitleSize(0.06);
  }

  void PhotonGBTFilterValidator::Run(hipo::banklist& banks) const
  {
    // get the particle bank
    auto& particle_bank = GetBank(banks, b_particle, "REC::Particle");
    
    std::vector<ROOT::Math::PxPyPzEVector> photons;
    for(int row = 0; row < particle_bank.getRows(); row++){
        auto pid = particle_bank.getInt("pid",row);
        if(pid!=22) continue;
        float px = particle_bank.getFloat("px",row);
        float py = particle_bank.getFloat("py",row);
        float pz = particle_bank.getFloat("pz",row);
        float E  = std::hypot(px,py,pz);
        ROOT::Math::PxPyPzEVector phot(px,py,pz,E);
        photons.push_back(phot);
    }

    // run the photon filter
    m_algo_seq->Run(banks);
      
    std::vector<ROOT::Math::PxPyPzEVector> filtered_photons;
    for(int row = 0; row < particle_bank.getRows(); row++){
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

  void PhotonGBTFilterValidator::FillHistograms(const std::vector<ROOT::Math::PxPyPzEVector>& photons, int idx) const
  {
    for (const auto& photon : photons) {
        h_P.at(idx)->Fill(photon.P());
        h_Th.at(idx)->Fill(photon.Theta() * 180.0 / 3.1415);
        h_Phi.at(idx)->Fill(photon.Phi() * 180.0 / 3.1415);
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
          gStyle->SetOptStat(0);
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
