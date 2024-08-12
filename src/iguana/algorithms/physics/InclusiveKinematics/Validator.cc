#include "Validator.h"
#include "iguana/services/LoggerMacros.h"

#include <Math/Vector3D.h>
#include <TStyle.h>

namespace iguana::physics {

  REGISTER_IGUANA_VALIDATOR(InclusiveKinematicsValidator);

  void InclusiveKinematicsValidator::Start(hipo::banklist& banks)
  {
    // define the algorithm sequence
    m_algo_seq = std::make_unique<AlgorithmSequence>();
    m_algo_seq->Add("physics::InclusiveKinematics");
    m_algo_seq->SetOption("physics::InclusiveKinematics", "log", GetLogLevel());
    m_algo_seq->Start(banks);

    // get bank indices
    b_particle = GetBankIndex(banks, "REC::Particle");
    b_result   = GetBankIndex(banks, "physics::InclusiveKinematics");

    // set an output file
    auto output_dir = GetOutputDirectory();
    if(output_dir) {
      m_output_file_basename = output_dir.value() + "/inclusive_kinematics";
      m_output_file          = new TFile(m_output_file_basename + ".root", "RECREATE");
    }

    // define plots
    // clang-format off
    gStyle->SetOptStat(0);
    const int n_bins = 100;
    lepton_p_dist     = new TH1D("lepton_p_dist",     "lepton p;p [GeV]",                    n_bins, 0,    12);
    lepton_theta_dist = new TH1D("lepton_theta_dist", "lepton #theta;#theta [deg]",          n_bins, 0,    60);
    lepton_phi_dist   = new TH1D("lepton_phi_dist",   "lepton #phi;#phi [deg]",              n_bins, -180, 180);
    lepton_vz_dist    = new TH1D("lepton_vz_dist",    "lepton v_{z};v_{z} [cm]",             n_bins, -30,  30);
    Q2_vs_x           = new TH2D("Q2_vs_x",           "Q^{2} vs. x;x;Q^{2} [GeV^{2}]",       n_bins, 0,    1, n_bins, 0, 12);
    Q2_vs_W           = new TH2D("Q2_vs_W",           "Q^{2} vs. W;W [GeV];Q^{2} [GeV^{2}]", n_bins, 0,    5, n_bins, 0, 12);
    y_dist            = new TH1D("y_dist",            "y distribution;y",                    n_bins, 0,    1);
    nu_dist           = new TH1D("nu_dist",           "#nu distribution;#nu",                n_bins, 0,    12);
    // clang-format on

    // format plots
    for(auto hist : {lepton_p_dist, lepton_theta_dist, lepton_phi_dist, lepton_vz_dist}) {
      hist->SetLineColor(kYellow + 2);
      hist->SetFillColor(kYellow + 2);
    }
    for(auto hist : {y_dist, nu_dist}) {
      hist->SetLineColor(kBlue);
      hist->SetFillColor(kBlue);
    }
  }


  void InclusiveKinematicsValidator::Run(hipo::banklist& banks) const
  {
    // calculate kinematics
    m_algo_seq->Run(banks);
    auto& particle_bank = GetBank(banks, b_particle, "REC::Particle");
    auto& result_bank   = GetBank(banks, b_result, "physics::InclusiveKinematics");

    if(result_bank.getRowList().size() == 0) {
      DEBUG("skip this event, since it has no inclusive kinematics results");
      return;
    }
    if(result_bank.getRowList().size() > 1) {
      WARN("found event with more than 1 inclusive kinematics bank rows; only the first row will be used");
    }

    auto pindex = result_bank.getShort("pindex", 0);
    auto Q2     = result_bank.getDouble("Q2", 0);
    auto x      = result_bank.getDouble("x", 0);
    auto W      = result_bank.getDouble("W", 0);
    auto y      = result_bank.getDouble("y", 0);
    auto nu     = result_bank.getDouble("nu", 0);

    ROOT::Math::XYZVector vec_lepton(
        particle_bank.getFloat("px", pindex),
        particle_bank.getFloat("py", pindex),
        particle_bank.getFloat("pz", pindex));
    auto lepton_p     = std::sqrt(vec_lepton.Mag2());
    auto lepton_theta = vec_lepton.Theta() * 180.0 / M_PI;
    auto lepton_phi   = vec_lepton.Phi() * 180.0 / M_PI;
    auto lepton_vz    = particle_bank.getFloat("vz", pindex);
    while(lepton_phi > 180)
      lepton_phi -= 360;
    while(lepton_phi < -180)
      lepton_phi += 360;

    // lock mutex and fill the plots
    std::scoped_lock<std::mutex> lock(m_mutex);
    lepton_p_dist->Fill(lepton_p);
    lepton_theta_dist->Fill(lepton_theta);
    lepton_phi_dist->Fill(lepton_phi);
    lepton_vz_dist->Fill(lepton_vz);
    Q2_vs_x->Fill(x, Q2);
    Q2_vs_W->Fill(W, Q2);
    y_dist->Fill(y);
    nu_dist->Fill(nu);
  }


  void InclusiveKinematicsValidator::Stop()
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
          lepton_p_dist->Draw();
          break;
        case 2:
          lepton_theta_dist->Draw();
          break;
        case 3:
          lepton_phi_dist->Draw();
          break;
        case 4:
          lepton_vz_dist->Draw();
          break;
        case 5:
          pad->SetLogz();
          Q2_vs_x->Draw("colz");
          break;
        case 6:
          pad->SetLogz();
          Q2_vs_W->Draw("colz");
          break;
        case 7:
          y_dist->Draw();
          break;
        case 8:
          nu_dist->Draw();
          break;
        }
      }
      canv->SaveAs(m_output_file_basename + ".png");
      m_output_file->Write();
      INFO("Wrote output file {}", m_output_file->GetName());
      m_output_file->Close();
    }
  }

}
