#include "Validator.h"
#include "TypeDefs.h"

#include <Math/Vector3D.h>
#include <TStyle.h>

namespace iguana::physics {

  REGISTER_IGUANA_VALIDATOR(DihadronKinematicsValidator);

  void DihadronKinematicsValidator::Start(hipo::banklist& banks)
  {
    // define the algorithm sequence
    m_algo_seq = std::make_unique<AlgorithmSequence>();
    m_algo_seq->Add("physics::DihadronKinematics");
    m_algo_seq->SetOption("physics::DihadronKinematics", "log", m_log->GetLevel());
    m_algo_seq->SetOption<std::vector<int>>("physics::DihadronKinematics", "hadron_a_list", {particle::pi_plus});
    m_algo_seq->SetOption<std::vector<int>>("physics::DihadronKinematics", "hadron_b_list", {particle::pi_minus});
    m_algo_seq->Start(banks);

    // get bank indices
    b_result   = GetBankIndex(banks, "physics::DihadronKinematics");

    // set an output file
    auto output_dir = GetOutputDirectory();
    if(output_dir) {
      m_output_file_basename = output_dir.value() + "/dihadron_kinematics";
      m_output_file          = new TFile(m_output_file_basename + ".root", "RECREATE");
    }

    // define plots
    // clang-format off
    gStyle->SetOptStat(0);
    const int n_bins = 100;
    Mh_dist = new TH1D("Mh_dist", "#pi^{+}#pi^{-} invariant mass M_{h};M_{h} [GeV]", n_bins, 0, 4);
    // clang-format on

    // format plots
    for(auto hist : {Mh_dist}) {
      hist->SetLineColor(kRed);
      hist->SetFillColor(kRed);
    }
  }


  void DihadronKinematicsValidator::Run(hipo::banklist& banks) const
  {
    // calculate kinematics
    m_algo_seq->Run(banks);
    auto& result_bank   = GetBank(banks, b_result, "physics::DihadronKinematics");

    if(result_bank.getRowList().size() == 0) {
      m_log->Debug("skip this event, since it has no kinematics results");
      return;
    }

    // lock mutex and fill the plots
    std::scoped_lock<std::mutex> lock(m_mutex);
    for(auto const& row : result_bank.getRowList()) {
      Mh_dist->Fill(result_bank.getDouble("Mh", row));
    }
  }


  void DihadronKinematicsValidator::Stop()
  {
    if(GetOutputDirectory()) {
      int n_rows = 2;
      int n_cols = 4;
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
          Mh_dist->Draw();
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
