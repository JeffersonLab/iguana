#include "Validator.h"
#include <TStyle.h>

namespace iguana::physics {

  REGISTER_IGUANA_VALIDATOR(DepolarizationValidator);

  void DepolarizationValidator::Start(hipo::banklist& banks)
  {
    // define the algorithm sequence
    m_algo_seq = std::make_unique<AlgorithmSequence>();
    m_algo_seq->Add("physics::InclusiveKinematics");
    m_algo_seq->Add("physics::Depolarization");
    m_algo_seq->SetOption("physics::Depolarization", "log", m_log->GetLevel());
    m_algo_seq->Start(banks);

    // get bank indices
    b_inc_kin = GetBankIndex(banks, "physics::InclusiveKinematics");
    b_depol   = GetBankIndex(banks, "physics::Depolarization");

    // set an output file
    auto output_dir = GetOutputDirectory();
    if(output_dir) {
      m_output_file_basename = output_dir.value() + "/depolarization";
      m_output_file          = new TFile(m_output_file_basename + ".root", "RECREATE");
    }

    // define plots
    gStyle->SetOptStat(0);
    int const n_bins = 100;

    double const depol_range[2] = {-2, 2};
    double const Q2_range[2]    = {0, 10};
    double const x_range[2]     = {0, 1};
    double const y_range[2]     = {0, 1};

    auto access_epsilon = [](auto const& b, auto const r) { return b.getDouble("epsilon", r); };
    auto access_A       = [](auto const& b, auto const r) { return b.getDouble("A", r); };
    auto access_B       = [](auto const& b, auto const r) { return b.getDouble("B", r); };
    auto access_C       = [](auto const& b, auto const r) { return b.getDouble("C", r); };
    auto access_V       = [](auto const& b, auto const r) { return b.getDouble("V", r); };
    auto access_W       = [](auto const& b, auto const r) { return b.getDouble("W", r); };

    plots_vs_Q2 = {
      { new TH2D("epsilon_vs_Q2", "#epsilon vs. Q^{2}", n_bins, Q2_range[0], Q2_range[1], n_bins, depol_range[0], depol_range[1]), access_epsilon },
      { new TH2D("A_vs_Q2", "A vs. Q^{2}", n_bins, Q2_range[0], Q2_range[1], n_bins, depol_range[0], depol_range[1]), access_A },
      { new TH2D("B_vs_Q2", "B vs. Q^{2}", n_bins, Q2_range[0], Q2_range[1], n_bins, depol_range[0], depol_range[1]), access_B },
      { new TH2D("C_vs_Q2", "C vs. Q^{2}", n_bins, Q2_range[0], Q2_range[1], n_bins, depol_range[0], depol_range[1]), access_C },
      { new TH2D("V_vs_Q2", "V vs. Q^{2}", n_bins, Q2_range[0], Q2_range[1], n_bins, depol_range[0], depol_range[1]), access_V },
      { new TH2D("W_vs_Q2", "W vs. Q^{2}", n_bins, Q2_range[0], Q2_range[1], n_bins, depol_range[0], depol_range[1]), access_W },
    };

    plots_vs_x = {
      { new TH2D("epsilon_vs_x", "#epsilon vs. x", n_bins, x_range[0], x_range[1], n_bins, depol_range[0], depol_range[1]), access_epsilon },
      { new TH2D("A_vs_x", "A vs. x", n_bins, x_range[0], x_range[1], n_bins, depol_range[0], depol_range[1]), access_A },
      { new TH2D("B_vs_x", "B vs. x", n_bins, x_range[0], x_range[1], n_bins, depol_range[0], depol_range[1]), access_B },
      { new TH2D("C_vs_x", "C vs. x", n_bins, x_range[0], x_range[1], n_bins, depol_range[0], depol_range[1]), access_C },
      { new TH2D("V_vs_x", "V vs. x", n_bins, x_range[0], x_range[1], n_bins, depol_range[0], depol_range[1]), access_V },
      { new TH2D("W_vs_x", "W vs. x", n_bins, x_range[0], x_range[1], n_bins, depol_range[0], depol_range[1]), access_W },
    };

    plots_vs_y = {
      { new TH2D("epsilon_vs_y", "#epsilon vs. y", n_bins, y_range[0], y_range[1], n_bins, depol_range[0], depol_range[1]), access_epsilon },
      { new TH2D("A_vs_y", "A vs. y", n_bins, y_range[0], y_range[1], n_bins, depol_range[0], depol_range[1]), access_A },
      { new TH2D("B_vs_y", "B vs. y", n_bins, y_range[0], y_range[1], n_bins, depol_range[0], depol_range[1]), access_B },
      { new TH2D("C_vs_y", "C vs. y", n_bins, y_range[0], y_range[1], n_bins, depol_range[0], depol_range[1]), access_C },
      { new TH2D("V_vs_y", "V vs. y", n_bins, y_range[0], y_range[1], n_bins, depol_range[0], depol_range[1]), access_V },
      { new TH2D("W_vs_y", "W vs. y", n_bins, y_range[0], y_range[1], n_bins, depol_range[0], depol_range[1]), access_W },
    };

  }


  void DepolarizationValidator::Run(hipo::banklist& banks) const
  {
    // calculate kinematics
    m_algo_seq->Run(banks);
    auto& inc_kin_bank = GetBank(banks, b_inc_kin, "physics::InclusiveKinematics");
    auto& depol_bank   = GetBank(banks, b_depol, "physics::Depolarization");

    // skip events with no hadrons
    if(inc_kin_bank.getRowList().size() == 0 || depol_bank.getRowList().size() == 0) {
      m_log->Debug("skip this event, since it has no kinematics results");
      return;
    }

    // lock mutex and fill the plots
    std::scoped_lock<std::mutex> lock(m_mutex);
    for(auto const& row : inc_kin_bank.getRowList()) {
      for(auto& plot : plots_vs_Q2)
        plot.hist->Fill(inc_kin_bank.getDouble("Q2", row), plot.get_val(depol_bank, row));
      for(auto& plot : plots_vs_x)
        plot.hist->Fill(inc_kin_bank.getDouble("x", row), plot.get_val(depol_bank, row));
      for(auto& plot : plots_vs_y)
        plot.hist->Fill(inc_kin_bank.getDouble("y", row), plot.get_val(depol_bank, row));
    }
  }


  void DepolarizationValidator::Stop()
  {
    if(GetOutputDirectory()) {
      std::vector<TCanvas*> canv_list;
      for(auto const& [name, plot_list] : std::map<TString,std::vector<Plot2D>>{{"Q2",plots_vs_Q2}, {"x",plots_vs_x}, {"y",plots_vs_y}}) {
        int const n_cols = 4;
        int const n_rows = (plot_list.size() - 1) / n_cols + 1;
        canv_list.push_back(new TCanvas("canv_" + name, name, n_cols * 800, n_rows * 600));
        canv_list.back()->Divide(n_cols, n_rows);
        int pad_num = 0;
        for(auto& plot : plot_list) {
          auto pad = canv_list.back()->GetPad(++pad_num);
          pad->cd();
          pad->SetGrid(1, 1);
          pad->SetLeftMargin(0.12);
          pad->SetRightMargin(0.12);
          pad->SetBottomMargin(0.12);
          plot.hist->Draw("colz");
        }
      }
      for(auto const& canv : canv_list)
        canv->SaveAs(m_output_file_basename + canv->GetTitle() + ".png");
      m_output_file->Write();
      m_log->Info("Wrote output file {}", m_output_file->GetName());
      m_output_file->Close();
    }
  }

}
