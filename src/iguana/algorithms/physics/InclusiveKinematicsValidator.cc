#include "InclusiveKinematicsValidator.h"

#include <TStyle.h>

namespace iguana::physics {

  REGISTER_IGUANA_VALIDATOR(InclusiveKinematicsValidator);

  void InclusiveKinematicsValidator::Start(hipo::banklist& banks)
  {
    // define the algorithm sequence
    m_algo_seq = std::make_unique<AlgorithmSequence>();
    m_algo_seq->Add("physics::InclusiveKinematics");
    m_algo_seq->Start(banks);

    // get bank indices
    b_result = GetBankIndex(banks, "physics::InclusiveKinematics");

    // set an output file
    auto output_dir = GetOutputDirectory();
    if(output_dir) {
      m_output_file_basename = output_dir.value() + "/inclusive_kinematics";
      m_output_file          = new TFile(m_output_file_basename + ".root", "RECREATE");
    }

    // define plots
    gStyle->SetOptStat(0);

    const int n_bins         = 100;
    const double range_x[2]  = {0, 1};
    const double range_Q2[2] = {0, 12};
    const double range_W[2]  = {0, 5};
    const double range_Y[2]  = {0, 1};
    const double range_Nu[2] = {0, 1};

    Q2_vs_x = new TH2D("Q2_vs_x", "Q^{2} vs. x;x;Q^{2} [GeV^{2}]", n_bins, range_x[0], range_x[1], n_bins, range_Q2[0], range_Q2[1]);
    Q2_vs_W = new TH2D("Q2_vs_W", "Q^{2} vs. W;W [GeV];Q^{2} [GeV^{2}]", n_bins, range_W[0], range_W[1], n_bins, range_Q2[0], range_Q2[1]);
    y_dist  = new TH1D("y_dist", "y distribution;y", n_bins, range_Y[0], range_Y[1]);
    nu_dist = new TH1D("nu_dist", "#nu distribution;#nu", n_bins, range_Nu[0], range_Nu[1]);
  }


  void InclusiveKinematicsValidator::Run(hipo::banklist& banks) const
  {
    // calculate kinematics
    m_algo_seq->Run(banks);
    auto& result_bank = GetBank(banks, b_result, "physics::InclusiveKinematics");

    auto Q2 = result_bank.getDouble("Q2", 0);
    auto x  = result_bank.getDouble("x", 0);
    auto W  = result_bank.getDouble("W", 0);
    auto y  = result_bank.getDouble("y", 0);
    auto nu = result_bank.getDouble("nu", 0);

    // lock mutex and fill the plots
    std::scoped_lock<std::mutex> lock(m_mutex);
    Q2_vs_x->Fill(x, Q2);
    Q2_vs_W->Fill(W, Q2);
    y_dist->Fill(y);
    nu_dist->Fill(nu);
  }


  void InclusiveKinematicsValidator::Stop()
  {
    if(GetOutputDirectory()) {
      int n_rows = 2;
      int n_cols = 2;
      auto canv  = new TCanvas("canv", "canv", n_cols * 800, n_rows * 600);
      canv->Divide(n_cols, n_rows);
      for(int pad_num = 1; pad_num <= n_rows * n_cols; pad_num++) {
        auto pad = canv->GetPad(pad_num);
        pad->cd();
        pad->SetGrid(1, 1);
        pad->SetLeftMargin(0.12);
        pad->SetRightMargin(0.12);
        pad->SetBottomMargin(0.12);
        switch(pad_num) {
        case 1:
          pad->SetLogz();
          Q2_vs_x->Draw("colz");
          break;
        case 2:
          pad->SetLogz();
          Q2_vs_W->Draw("colz");
          break;
        case 3:
          y_dist->Draw();
          break;
        case 4:
          nu_dist->Draw();
          break;
        }
      }
      canv->SaveAs(m_output_file_basename + ".png");
      m_output_file->Write();
      m_log->Info("Wrote output file {}", m_output_file->GetName());
      m_output_file->Close();
    }
  }

}
