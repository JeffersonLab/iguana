#include "Validator.h"
#include "TypeDefs.h"

#include <Math/Vector3D.h>
#include <TStyle.h>

namespace iguana::physics {

  REGISTER_IGUANA_VALIDATOR(SingleHadronKinematicsValidator);

  void SingleHadronKinematicsValidator::Start(hipo::banklist& banks)
  {
    // define the algorithm sequence
    m_algo_seq = std::make_unique<AlgorithmSequence>();
    m_algo_seq->Add("physics::InclusiveKinematics");
    m_algo_seq->Add("physics::SingleHadronKinematics");
    m_algo_seq->SetOption("physics::SingleHadronKinematics", "log", m_log->GetLevel());
    m_algo_seq->SetOption<std::vector<int>>("physics::SingleHadronKinematics", "hadron_list", {particle::pi_plus});
    m_algo_seq->Start(banks);

    // get bank indices
    b_result = GetBankIndex(banks, "physics::SingleHadronKinematics");

    // set an output file
    auto output_dir = GetOutputDirectory();
    if(output_dir) {
      m_output_file_basename = output_dir.value() + "/single_hadron_kinematics";
      m_output_file          = new TFile(m_output_file_basename + ".root", "RECREATE");
    }

    // define plots
    gStyle->SetOptStat(0);
    const int n_bins = 100;
    plot_list = {
      {
        new TH1D("z_dist", "z", n_bins, 0, 1),
        [](auto const& b, auto const r) { return b.getDouble("z", r); }
      },
      {
        new TH1D("PhPerp_dist", "P_{h}^{{}^{#perp}}", n_bins, 0, 2),
        [](auto const& b, auto const r) { return b.getDouble("PhPerp", r); }
      },
      {
        new TH1D("MX_dist", "missing mass M_{X} [GeV];", n_bins, 0, 4),
        [](auto const& b, auto const r) { auto MX2 = b.getDouble("MX2", r); return MX2 >= 0 ? std::sqrt(MX2) : -100; } // FIXME: handle space-like case better
      },
      {
        new TH1D("xF_dist", "x_{F};", n_bins, -1, 1),
        [](auto const& b, auto const r) { return b.getDouble("xF", r); }
      },
      {
        new TH1D("phiH_dist", "#phi_{h};", n_bins, -M_PI, M_PI),
        [](auto const& b, auto const r) { return b.getDouble("phiH", r); }
      },
      {
        new TH1D("xi_dist", "#xi", n_bins, -1, 1),
        [](auto const& b, auto const r) { return b.getDouble("xi", r); }
      },
    };

    // format plots
    for(auto& plot : plot_list) {
      plot.hist->SetLineColor(kGreen+1);
      plot.hist->SetFillColor(kGreen+1);
        plot.hist->SetTitle(
          TString(particle::title.at(particle::pi_plus)) +
          " " + plot.hist->GetTitle());
    }
  }


  void SingleHadronKinematicsValidator::Run(hipo::banklist& banks) const
  {
    // calculate kinematics
    m_algo_seq->Run(banks);
    auto& result_bank   = GetBank(banks, b_result, "physics::SingleHadronKinematics");

    // skip events with no hadrons
    if(result_bank.getRowList().size() == 0) {
      m_log->Debug("skip this event, since it has no kinematics results");
      return;
    }

    // lock mutex and fill the plots
    std::scoped_lock<std::mutex> lock(m_mutex);
    for(auto const& row : result_bank.getRowList()) {
      for(auto& plot : plot_list)
        plot.hist->Fill(plot.get_val(result_bank, row));
    }
  }


  void SingleHadronKinematicsValidator::Stop()
  {
    if(GetOutputDirectory()) {
      int const n_cols = 4;
      int const n_rows = (plot_list.size() - 1) / n_cols + 1;
      auto canv  = new TCanvas("canv", "canv", n_cols * 800, n_rows * 600);
      canv->Divide(n_cols, n_rows);
      int pad_num = 0;
      for(auto& plot : plot_list) {
        auto pad = canv->GetPad(++pad_num);
        pad->cd();
        pad->SetGrid(1, 1);
        pad->SetLeftMargin(0.12);
        pad->SetRightMargin(0.12);
        pad->SetBottomMargin(0.12);
        plot.hist->Draw();
      }
      canv->SaveAs(m_output_file_basename + ".png");
      m_output_file->Write();
      m_log->Info("Wrote output file {}", m_output_file->GetName());
      m_output_file->Close();
    }
  }

}
