#include "Validator.h"

#include <TCanvas.h>
#include <TLegend.h>
#include <unordered_set>

namespace iguana::clas12 {

  REGISTER_IGUANA_VALIDATOR(RGAFiducialFilterValidator);

  // static helpers
  int RGAFiducialFilterValidator::LayerToIndex(int layer) {
    if (layer == 1) return 0; // PCAL
    if (layer == 4) return 1; // ECIN
    if (layer == 7) return 2; // ECOUT
    return -1;
  }
  const char* RGAFiducialFilterValidator::LayerName(int i) {
    static const char* names[3] = {"PCAL","ECIN","ECOUT"};
    return (i>=0 && i<3) ? names[i] : "UNK";
  }
  const char* RGAFiducialFilterValidator::AxisName(int i) {
    static const char* names[3] = {"lu","lv","lw"};
    return (i>=0 && i<3) ? names[i] : "ax";
  }

  void RGAFiducialFilterValidator::BookPlotsForPID(int pid)
  {
    // allocate histograms if not present
    auto& grid = u_plots[pid];

    // x-range 0..400, 200 bins
    const int    nbins = 90;
    const double xmin  = 0.0;
    const double xmax  = 405.0;

    for (int L = 0; L < 3; ++L) {        // layer index
      for (int A = 0; A < 3; ++A) {      // axis index  (0=lu,1=lv,2=lw)
        for (int s = 1; s <= 6; ++s) {   // sector 1..6
          TH1D*& h =
            (A == 0) ? grid.layer[L].lu.sec[s] :
            (A == 1) ? grid.layer[L].lv.sec[s] :
                       grid.layer[L].lw.sec[s];

          if (!h) {
            TString hname = Form("h_%d_%s_%s_s%d", pid, LayerName(L), AxisName(A), s);
            TString htitle = Form("%s %s (pid %d, sector %d);%s;counts",
                                  LayerName(L), AxisName(A), pid, s, AxisName(A));
            h = new TH1D(hname, htitle, nbins, xmin, xmax);
            h->SetLineWidth(2);
          }
        }
      }
    }
  }

  void RGAFiducialFilterValidator::Start(hipo::banklist& banks)
  {
    // Build sequence: run fiducial filter, then plot surviving tracks
    m_algo_seq = std::make_unique<AlgorithmSequence>();
    m_algo_seq->Add("clas12::RGAFiducialFilter");
    m_algo_seq->Start(banks);

    // banks
    b_particle = GetBankIndex(banks, "REC::Particle");
    b_calor    = GetBankIndex(banks, "REC::Calorimeter");

    // output file (optional)
    auto output_dir = GetOutputDirectory();
    if (output_dir) {
      m_output_file_basename = output_dir.value() + "/rga_fiducial_calorimeter";
      m_output_file          = new TFile(m_output_file_basename + ".root", "RECREATE");
    }

    // book plots for both PIDs we care about
    for (auto pid : u_pid_list) BookPlotsForPID(pid);
  }

  void RGAFiducialFilterValidator::Run(hipo::banklist& banks) const
  {
    auto& particle_bank = GetBank(banks, b_particle, "REC::Particle");
    auto& calor_bank    = GetBank(banks, b_calor,    "REC::Calorimeter");

    // Run the filter first; plot hits associated with surviving tracks only
    m_algo_seq->Run(banks);

    // Build survivor sets keyed by pid 
    std::unordered_map<int, std::unordered_set<int>> survivors;
    for (auto pid : u_pid_list) survivors[pid]; // ensure keys exist

    for (auto const& row : particle_bank.getRowList()) {
      const int pid = particle_bank.getInt("pid", row);
      if (survivors.find(pid) != survivors.end()) {
        survivors[pid].insert(static_cast<int>(row));
      }
    }

    // Lock while filling hists
    std::scoped_lock<std::mutex> lock(m_mutex);

    // One pass over calorimeter bank; fill when pindex belongs to a surviving track of interest
    const int nrows = calor_bank.getRows();
    for (int i = 0; i < nrows; ++i) {
      const int pindex = calor_bank.getInt("pindex", i);
      const int layer  = calor_bank.getInt("layer",  i);
      const int sector = calor_bank.getInt("sector", i);
      const int L      = LayerToIndex(layer);
      if (L < 0 || sector < 1 || sector > 6) continue;

      const float lu = calor_bank.getFloat("lu", i);
      const float lv = calor_bank.getFloat("lv", i);
      const float lw = calor_bank.getFloat("lw", i);

      // Fill for each pid type where this pindex survived
      for (auto pid : u_pid_list) {
        auto it = survivors.find(pid);
        if (it == survivors.end()) continue;
        if (it->second.find(pindex) == it->second.end()) continue;

        // plots exist from Start()
        auto& grid = const_cast<RGAFiducialFilterValidator*>(this)->u_plots[pid];
        grid.layer[L].lu.sec[sector]->Fill(lu);
        grid.layer[L].lv.sec[sector]->Fill(lv);
        grid.layer[L].lw.sec[sector]->Fill(lw);
      }
    }
  }

  void RGAFiducialFilterValidator::DrawCanvasForPID(int pid)
  {
    auto find_color = [](int sector) {
      static const int colors[7] = {
        0, kRed+1, kBlue+1, kGreen+2, kMagenta+1, kCyan+1, kOrange+1
      };
      return colors[sector];
    };

    auto& grid = u_plots.at(pid);

    // 3x3: rows=lu,lv,lw (0..2), cols=PCAL,ECIN,ECOUT (0..2)
    TString canv_name = Form("rgafid_pid%d", pid);
    auto canv = new TCanvas(canv_name, canv_name, 1500, 1200);
    canv->Divide(3,3);

    for (int row = 0; row < 3; ++row) {        // 0=lu,1=lv,2=lw
      for (int col = 0; col < 3; ++col) {      // 0=PCAL,1=ECIN,2=ECOUT
        const int pad = row*3 + col + 1;
        canv->cd(pad);

        TH1D* href = nullptr;
        auto leg = new TLegend(0.65, 0.70, 0.88, 0.90);
        leg->SetBorderSize(0);
        leg->SetFillStyle(0);
        leg->SetTextSize(0.03);

        for (int s = 1; s <= 6; ++s) {
          TH1D* h =
            (row == 0) ? grid.layer[col].lu.sec[s] :
            (row == 1) ? grid.layer[col].lv.sec[s] :
                         grid.layer[col].lw.sec[s];

          if (!h) continue;
          h->SetLineColor(find_color(s));
          h->SetLineWidth(2);
          if (!href) {
            TString title = Form("%s %s (pid %d);%s;counts",
                                 LayerName(col), AxisName(row), pid, AxisName(row));
            h->SetTitle(title);
            h->Draw("hist");
            href = h;
          } else {
            h->Draw("hist same");
          }
          leg->AddEntry(h, Form("sector %d", s), "l");
        }

        if (href) leg->Draw();
      }
    }

    TString png_name = Form("%s_pid%d.png", m_output_file_basename.Data(), pid);
    canv->SaveAs(png_name);
  }

  void RGAFiducialFilterValidator::Stop()
  {
    // Draw and save two canvases: pid 11 and pid 22
    if (GetOutputDirectory()) {
      for (auto pid : u_pid_list) {
        DrawCanvasForPID(pid);
      }
      if (m_output_file) {
        m_output_file->Write();
        m_log->Info("Wrote output file {}", m_output_file->GetName());
        m_output_file->Close();
      }
    }
  }

} // namespace iguana::clas12