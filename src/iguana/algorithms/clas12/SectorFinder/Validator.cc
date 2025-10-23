#include "Validator.h"

#include <TProfile.h>

namespace iguana::clas12 {

  REGISTER_IGUANA_VALIDATOR(SectorFinderValidator);

  void SectorFinderValidator::Start(hipo::banklist& banks)
  {
    // define the algorithm sequence
    m_algo_seq = std::make_unique<AlgorithmSequence>();
    m_algo_seq->Add("clas12::EventBuilderFilter");
    m_algo_seq->Add("clas12::SectorFinder");
    m_algo_seq->SetOption<std::vector<int>>("clas12::EventBuilderFilter", "pids", u_pdg_list);
    m_algo_seq->SetOption<std::string>("clas12::SectorFinder", "bank_charged", "REC::Track");
    m_algo_seq->SetOption<std::string>("clas12::SectorFinder", "bank_uncharged", "default");
    m_algo_seq->Start(banks);


    b_particle = GetBankIndex(banks, "REC::Particle");
    b_cal      = GetBankIndex(banks, "REC::Calorimeter");
    b_sector   = GetBankIndex(banks, "REC::Particle::Sector");

    // set an output file
    auto output_dir = GetOutputDirectory();
    if(output_dir) {
      m_output_file_basename = output_dir.value() + "/sector_finder";
      m_output_file          = new TFile(m_output_file_basename + ".root", "RECREATE");
    }

    // define plots
    for(auto const& pdg : u_pdg_list) {
      std::vector<TH2D*> YvsX;
      TString particle_name  = particle::name.at(particle::PDG(pdg));
      TString particle_title = particle::title.at(particle::PDG(pdg));
      for(int sec = 1; sec <= 6; sec++) {
        TString sector_name  = Form("sec%d", sec);
        TString sector_title = Form("sector %d", sec);
        YvsX.push_back(new TH2D(
            "YvsX_" + particle_name + "_" + sector_name,
            particle_title + " Calorimeter Hit Position, " + sector_title + ";X [cm];Y [cm]",
            50, -500, 500,
            50, -500, 500));
      }
      u_YvsX.insert({pdg, YvsX});
    }
    u_IsInFD = new TH1D(
        "IsInFD",
        "e^{-} with #theta>6.5^{o} Sector; e^{-} Sector",
        7, -0.5, 6.5);
  }

  bool SectorFinderValidator::Run(hipo::banklist& banks) const
  {

    auto& particle_bank = GetBank(banks, b_particle, "REC::Particle");
    auto& sector_bank   = GetBank(banks, b_sector, "REC::Particle::Sector");
    auto& cal_bank      = GetBank(banks, b_cal, "REC::Calorimeter");


    m_algo_seq->Run(banks);

    // lock the mutex, so we can mutate plots
    std::scoped_lock<std::mutex> lock(m_mutex);
    // fill the plots
    for(auto const& row : particle_bank.getRowList()) {

      auto pdg    = particle_bank.getInt("pid", row);
      auto sector = sector_bank.getInt("sector", row);

      double x = 0, y = 0;
      for(auto const& rowcal : cal_bank.getRowList()) {
        if(cal_bank.getShort("pindex", rowcal) == row) {
          x = cal_bank.getFloat("x", rowcal);
          y = cal_bank.getFloat("y", rowcal);
        }
      }

      if(pdg == 11) {
        double Px    = particle_bank.getFloat("px", row);
        double Py    = particle_bank.getFloat("py", row);
        double Pz    = particle_bank.getFloat("pz", row);
        double P     = sqrt(Px * Px + Py * Py + Pz * Pz);
        double Theta = acos(Pz / P) * 180. / M_PI;
        // electrons are in FT or FD
        // sector should always be 1 if theta is larger than 5.5 degrees
        if(Theta > 6.5) {
          u_IsInFD->Fill(sector);
          if(sector == 0) {
            m_log->Trace("e' with theta={} and sector==0, this should not happen", Theta);
          }
        }
      }

      // skip central particle, or unknown sector
      if(!IsValidSector(sector))
        continue;
      m_log->Trace("Filling SectorFinder Validator, pdg {} sector {} pindex {}", pdg, sector, row);
      u_YvsX.at(pdg).at(sector - 1)->Fill(x, y);
    }

    return true;
  }


  void SectorFinderValidator::Stop()
  {
    if(GetOutputDirectory()) {
      for(auto const& [pdg, plots] : u_YvsX) {
        int n_cols        = 3;
        int n_rows        = 2;
        TString canv_name = Form("canv%d", pdg);
        auto canv         = new TCanvas(canv_name, canv_name, n_cols * 800, n_rows * 600);
        canv->Divide(n_cols, n_rows);
        int pad_num = 0;
        for(auto const& plot : plots) {
          auto pad = canv->GetPad(++pad_num);
          pad->cd();
          pad->SetGrid(1, 1);
          pad->SetLeftMargin(0.12);
          pad->SetRightMargin(0.12);
          pad->SetBottomMargin(0.12);
          plot->Draw("colz");
        }
        canv->SaveAs(m_output_file_basename + "_" + std::to_string(pdg) + ".png");
      }

      auto canv1D = new TCanvas("1D canvas", "1D canvas", 800, 600);
      u_IsInFD->Draw();
      canv1D->SaveAs(m_output_file_basename + "_elIsInFD.png");

      m_output_file->Write();
      m_log->Info("Wrote output file {}", m_output_file->GetName());
      m_output_file->Close();
    }
  }

}
