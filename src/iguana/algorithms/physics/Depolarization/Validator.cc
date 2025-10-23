#include "Validator.h"

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

    // plot binning
    int const n_bins                                             = 100;
    std::pair<double, double> const depol_range                  = {-0.5, 2.5};
    std::map<TString, std::pair<double, double>> const kin_range = {
        {"Q2", {0, 10}},
        {"x", {0, 1}},
        {"y", {0, 1}}};

    // plot names, in preferred order for the canvas
    std::vector<TString> const depol_names = {
        "A",
        "B",
        "C",
        "V",
        "W",
        "epsilon",
        "BA",
        "CA",
        "VA",
        "WA"};

    // plot titles
    std::map<TString, TString> const depol_titles = {
        {"epsilon", "#varepsilon"},
        {"A", "A"},
        {"B", "B"},
        {"C", "C"},
        {"V", "V"},
        {"W", "W"},
        {"BA", "B/A"},
        {"CA", "C/A"},
        {"VA", "V/A"},
        {"WA", "W/A"}};

    // bank accessors
    std::map<TString, std::function<double(hipo::bank const&, int const)>> const accessors = {
        {"epsilon", [](auto const& b, auto const r) { return b.getDouble("epsilon", r); }},
        {"A", [](auto const& b, auto const r) { return b.getDouble("A", r); }},
        {"B", [](auto const& b, auto const r) { return b.getDouble("B", r); }},
        {"C", [](auto const& b, auto const r) { return b.getDouble("C", r); }},
        {"V", [](auto const& b, auto const r) { return b.getDouble("V", r); }},
        {"W", [](auto const& b, auto const r) { return b.getDouble("W", r); }},
        {"BA", [](auto const& b, auto const r) { return b.getDouble("B", r) / b.getDouble("A", r); }},
        {"CA", [](auto const& b, auto const r) { return b.getDouble("C", r) / b.getDouble("A", r); }},
        {"VA", [](auto const& b, auto const r) { return b.getDouble("V", r) / b.getDouble("A", r); }},
        {"WA", [](auto const& b, auto const r) { return b.getDouble("W", r) / b.getDouble("A", r); }}};

    // construct plots
    for(auto const& name : depol_names) {
      plots_vs_Q2.push_back({new TH2D(name + "_vs_Q2", depol_titles.at(name) + " vs. Q^{2}", n_bins, kin_range.at("Q2").first, kin_range.at("Q2").second, n_bins, depol_range.first, depol_range.second),
                             accessors.at(name)});
      plots_vs_x.push_back({new TH2D(name + "_vs_x", depol_titles.at(name) + " vs. x", n_bins, kin_range.at("x").first, kin_range.at("x").second, n_bins, depol_range.first, depol_range.second),
                            accessors.at(name)});
      plots_vs_y.push_back({new TH2D(name + "_vs_y", depol_titles.at(name) + " vs. y", n_bins, kin_range.at("y").first, kin_range.at("y").second, n_bins, depol_range.first, depol_range.second),
                            accessors.at(name)});
    }
  }


  bool DepolarizationValidator::Run(hipo::banklist& banks) const
  {
    // calculate kinematics
    m_algo_seq->Run(banks);
    auto& inc_kin_bank = GetBank(banks, b_inc_kin, "physics::InclusiveKinematics");
    auto& depol_bank   = GetBank(banks, b_depol, "physics::Depolarization");

    // skip events with empty bank(s)
    if(inc_kin_bank.getRowList().size() == 0 || depol_bank.getRowList().size() == 0) {
      m_log->Debug("skip this event, since it has no kinematics results");
      return false;
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
    return true;
  }


  void DepolarizationValidator::Stop()
  {
    if(GetOutputDirectory()) {

      auto make_plots = [this](TString const& name, std::vector<Plot2D> const& plot_list) {
        int const n_cols = 5;
        int const n_rows = (plot_list.size() - 1) / n_cols + 1;
        auto canv        = new TCanvas("canv_" + name, name, n_cols * 800, n_rows * 1000);
        canv->Divide(n_cols, n_rows);
        int pad_num = 0;
        for(auto& plot : plot_list) {
          auto pad = canv->GetPad(++pad_num);
          pad->cd();
          pad->SetGrid(1, 1);
          pad->SetLeftMargin(0.12);
          pad->SetRightMargin(0.15);
          pad->SetBottomMargin(0.12);
          plot.hist->Draw("colz");
        }
        canv->SaveAs(m_output_file_basename + "_" + canv->GetTitle() + ".png");
      };

      make_plots("Q2", plots_vs_Q2);
      make_plots("x", plots_vs_x);
      make_plots("y", plots_vs_y);

      m_output_file->Write();
      m_log->Info("Wrote output file {}", m_output_file->GetName());
      m_output_file->Close();
    }
  }

}
