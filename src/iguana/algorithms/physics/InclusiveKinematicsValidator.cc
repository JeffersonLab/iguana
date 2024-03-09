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
    Q2vsX = new TH2D("Q2vsX", "Q^{2} vs. x;Q^{2} [GeV^{2}];x", 100, 0, 1, 100, 0, 12);
  }


  void InclusiveKinematicsValidator::Run(hipo::banklist& banks) const
  {
    // calculate kinematics
    m_algo_seq->Run(banks);
    auto& result_bank = GetBank(banks, b_result, "physics::InclusiveKinematics");

    // lock the mutex, so we can mutate plots
    std::scoped_lock<std::mutex> lock(m_mutex);

    // fill the plots
    auto Q2 = result_bank.getDouble("Q2", 0);
    auto x  = result_bank.getDouble("x", 0);
    Q2vsX->Fill(x, Q2);
  }


  void InclusiveKinematicsValidator::Stop()
  {
    if(GetOutputDirectory()) {
      auto canv = new TCanvas("canv", "canv", 800, 600);
      canv->SetGrid(1, 1);
      canv->SetLogz();
      Q2vsX->Draw("colz");
      canv->SaveAs(m_output_file_basename + "Q2vsX.png");
      m_output_file->Write();
      m_log->Info("Wrote output file {}", m_output_file->GetName());
      m_output_file->Close();
    }
  }

}
