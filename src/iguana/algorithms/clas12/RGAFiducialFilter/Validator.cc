#include "Validator.h"

#include <TCanvas.h>
#include <TPad.h>
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
  const char* RGAFiducialFilterValidator::PIDName(int pid) {
    return (pid == 11) ? "Electrons" : (pid == 22) ? "Photons" : "PID";
  }

  void RGAFiducialFilterValidator::BookPlotsForPID(int pid)
  {
    auto& sets = u_plots2d[pid];

    // 90 bins × 4.5 cm = 405 cm range
    const int    nb = 90;
    const double lo = 0.0;
    const double hi = 405.0;

    for (int L = 0; L < 3; ++L) {        // 0:PCAL 1:ECIN 2:ECOUT
      for (int s = 1; s <= 6; ++s) {     // sectors 1..6

        // lv vs lw
        if (!sets.layer[L].lv_lw.sec[s]) {
          TString hname  = Form("h2_%d_%s_lv_lw_s%d", pid, LayerName(L), s);
          TString htitle = Form("%s %s Sector %d;lv (cm);lw (cm)",
                                PIDName(pid), LayerName(L), s);
          auto* h = new TH2D(hname, htitle, nb, lo, hi, nb, lo, hi);
          h->SetStats(0);
          h->GetXaxis()->SetTitleOffset(1.1);
          h->GetYaxis()->SetTitleOffset(1.25);
          sets.layer[L].lv_lw.sec[s] = h;
        }

        // lv vs lu
        if (!sets.layer[L].lv_lu.sec[s]) {
          TString hname  = Form("h2_%d_%s_lv_lu_s%d", pid, LayerName(L), s);
          TString htitle = Form("%s %s Sector %d;lv (cm);lu (cm)",
                                PIDName(pid), LayerName(L), s);
          auto* h = new TH2D(hname, htitle, nb, lo, hi, nb, lo, hi);
          h->SetStats(0);
          h->GetXaxis()->SetTitleOffset(1.1);
          h->GetYaxis()->SetTitleOffset(1.25);
          sets.layer[L].lv_lu.sec[s] = h;
        }
      }
    }
  }

  void RGAFiducialFilterValidator::Start(hipo::banklist& banks)
  {
    // Build sequence: run fiducial filter, then plot surviving tracks
    m_algo_seq = std::make_unique<AlgorithmSequence>();
    m_algo_seq->Add("clas12::RGAFiducialFilter");
    // No strictness options here; algorithm defaults to 1 and ignores SetOption

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
    for (auto pid : u_pid_list) survivors[pid];

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

        auto& sets = const_cast<RGAFiducialFilterValidator*>(this)->u_plots2d[pid];
        sets.layer[L].lv_lw.sec[sector]->Fill(lv, lw);
        sets.layer[L].lv_lu.sec[sector]->Fill(lv, lu);
      }
    }
  }

  // Draw a 2×3 sector grid for one layer and one projection
  void RGAFiducialFilterValidator::DrawSectorGrid2D(int pid, int layer_idx, bool lv_vs_lw)
  {
    auto& sets = u_plots2d.at(pid);
    const char* proj = lv_vs_lw ? "lv_vs_lw" : "lv_vs_lu";

    TString canv_name  = Form("rgafid_%s_%s_pid%d", LayerName(layer_idx), proj, pid);
    TString canv_title = Form("%s %s — %s", PIDName(pid), LayerName(layer_idx),
                              lv_vs_lw ? "lv vs lw" : "lv vs lu");
    auto* canv = new TCanvas(canv_name, canv_title, 1400, 900);
    canv->Divide(3, 2);

    for (int s = 1; s <= 6; ++s) {
      canv->cd(s);
      // extra margins so labels aren't clipped; more right margin for the palette
      gPad->SetLeftMargin(0.16);
      gPad->SetRightMargin(0.14);
      gPad->SetBottomMargin(0.12);
      gPad->SetTopMargin(0.08);

      TH2D* h = lv_vs_lw ? sets.layer[layer_idx].lv_lw.sec[s]
                         : sets.layer[layer_idx].lv_lu.sec[s];
      if (!h) continue;

      TString t = Form("%s %s Sector %d", PIDName(pid), LayerName(layer_idx), s);
      h->SetTitle(t);
      h->Draw("COLZ");
    }

    TString png_name = Form("%s_%s_%s_pid%d.png",
                            m_output_file_basename.Data(),
                            LayerName(layer_idx), proj, pid);
    canv->SaveAs(png_name);
  }

  void RGAFiducialFilterValidator::Stop()
  {
    if (!GetOutputDirectory()) return;

    // For each PID and each layer, draw both projections
    for (auto pid : u_pid_list) {
      for (int L = 0; L < 3; ++L) {
        DrawSectorGrid2D(pid, L, true ); // lv vs lw
        DrawSectorGrid2D(pid, L, false); // lv vs lu
      }
    }

    if (m_output_file) {
      m_output_file->Write();
      m_log->Info("Wrote output file {}", m_output_file->GetName());
      m_output_file->Close();
    }
  }

} // namespace iguana::clas12