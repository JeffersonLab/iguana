#include "MomentumCorrectionValidator.h"

#include <TProfile.h>
#include <TStyle.h>

namespace iguana::clas12 {

  REGISTER_IGUANA_VALIDATOR(MomentumCorrectionValidator);

  void MomentumCorrectionValidator::Start(hipo::banklist& banks)
  {
    // define the algorithm sequence
    m_algo_seq = std::make_unique<AlgorithmSequence>();
    m_algo_seq->Add("clas12::EventBuilderFilter");
    m_algo_seq->Add("clas12::MomentumCorrection");
    m_algo_seq->SetOption<std::vector<int>>("clas12::EventBuilderFilter", "pids", u_pdg_list);
    m_algo_seq->Start(banks);

    // define the sector finder
    m_sector_finder = std::make_unique<SectorFinder>();
    m_sector_finder->Start(banks);

    // get bank indices
    b_particle = GetBankIndex(banks, "REC::Particle");

    // set an output file
    auto output_dir = GetOutputDirectory();
    if(output_dir) {
      m_output_file_basename = output_dir.value() + "/momentum_corrections";
      m_output_file          = new TFile(m_output_file_basename + ".root", "RECREATE");
    }

    // define plots
    gStyle->SetOptStat(0);
    for(const auto& pdg : u_pdg_list) {
      std::vector<TH2D*> deltaPvsP;
      TString particle_name  = particle::name.at(particle::PDG(pdg));
      TString particle_title = particle::title.at(particle::PDG(pdg));
      for(int sec = 1; sec <= 6; sec++) {
        TString sector_name  = Form("sec%d", sec);
        TString sector_title = Form("sector %d", sec);
        deltaPvsP.push_back(new TH2D(
            "deltaPvsP_" + particle_name + "_" + sector_name,
            particle_title + " momentum correction, " + sector_title + ";p [GeV];#Delta p [GeV]",
            30, 0, m_p_max,
            100, -m_deltaP_max, m_deltaP_max));
      }
      u_deltaPvsP.insert({pdg, deltaPvsP});
    }
  }


  void MomentumCorrectionValidator::Run(hipo::banklist& banks) const
  {
    // get the momenta before
    // FIXME: will need to refactor this once we have HIPO iterators
    auto& particle_bank = GetBank(banks, b_particle, "REC::Particle");
    std::unordered_map<int, double> p_measured;
    for(int row = 0; row < particle_bank.getRows(); row++) {
      double mom = std::hypot(
          particle_bank.getFloat("px", row),
          particle_bank.getFloat("py", row),
          particle_bank.getFloat("pz", row));
      p_measured.insert({row, mom});
    }

    // run the momentum corrections
    m_algo_seq->Run(banks);

    // get the sectors
    auto sectors = m_sector_finder->Find(banks);

    // lock the mutex, so we can mutate plots
    std::scoped_lock<std::mutex> lock(m_mutex);

    // fill the plots
    for(int row = 0; row < particle_bank.getRows(); row++) {

      auto pdg    = particle_bank.getInt("pid", row);
      auto sector = sectors.at(row);
      if(pdg == -1)
        continue; // FIXME: will need to refactor this once we have HIPO iterators

      // skip central particle, or unknown sector
      if(sector==0)
        continue;

      double p_corrected = std::hypot(
          particle_bank.getFloat("px", row),
          particle_bank.getFloat("py", row),
          particle_bank.getFloat("pz", row));
      auto delta_p = p_corrected - p_measured.at(row);
      u_deltaPvsP.at(pdg).at(sector-1)->Fill(p_corrected, delta_p);
    }
  }


  void MomentumCorrectionValidator::Stop()
  {
    if(GetOutputDirectory()) {
      for(const auto& [pdg, plots] : u_deltaPvsP) {
        int n_cols = 3;
        int n_rows = 2;
        TString canv_name = Form("canv%d", pdg);
        auto canv         = new TCanvas(canv_name, canv_name, n_cols * 800, n_rows * 600);
        canv->Divide(n_cols, n_rows);
        int pad_num = 0;
        for(const auto& plot : plots) {
          auto pad = canv->GetPad(++pad_num);
          pad->cd();
          pad->SetGrid(1, 1);
          pad->SetLogz();
          pad->SetLeftMargin(0.12);
          pad->SetRightMargin(0.12);
          pad->SetBottomMargin(0.12);
          plot->Draw("colz");
          plot->GetYaxis()->SetRangeUser(-m_deltaP_zoom, m_deltaP_zoom);
          auto prof = plot->ProfileX("_pfx", 1, -1, "s");
          prof->SetLineColor(kBlack);
          prof->SetLineWidth(5);
          prof->Draw("same");
        }
        canv->SaveAs(m_output_file_basename + "_" + std::to_string(pdg) + ".png");
      }
      m_output_file->Write();
      m_log->Info("Wrote output file {}", m_output_file->GetName());
      m_output_file->Close();
    }
  }

}
