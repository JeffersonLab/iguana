#include "Validator.h"

#include <TCanvas.h>
#include <TStyle.h>
#include <unordered_set>
#include <cstdlib>     // std::getenv
#include <string>      // std::stoi
#include <algorithm>   // std::min, std::max

namespace iguana::clas12 {

  REGISTER_IGUANA_VALIDATOR(RGAFiducialFilterValidator);

  // --- static helpers ---
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

  // book 2D histos per PID / layer / sector
  void RGAFiducialFilterValidator::BookPlotsForPID(int pid)
  {
    auto& grid = u_plots[pid];

    // 0..405 with 90 bins (4.5 cm/bin)
    const int    nbins = 90;
    const double xmin  = 0.0;
    const double xmax  = 405.0;

    for (int L = 0; L < 3; ++L) {          // layer index
      for (int s = 1; s <= 6; ++s) {       // sector 1..6
        if (!grid.layer[L].lv_vs_lw[s]) {
          TString hname  = Form("h2_%d_%s_lv_lw_s%d", pid, LayerName(L), s);
          TString htitle = Form("%s %s — Sector %d;lv [cm];lw [cm]",
                                PIDName(pid), LayerName(L), s);
          grid.layer[L].lv_vs_lw[s] = new TH2D(hname, htitle, nbins, xmin, xmax, nbins, xmin, xmax);
        }
        if (!grid.layer[L].lv_vs_lu[s]) {
          TString hname  = Form("h2_%d_%s_lv_lu_s%d", pid, LayerName(L), s);
          TString htitle = Form("%s %s — Sector %d;lv [cm];lu [cm]",
                                PIDName(pid), LayerName(L), s);
          grid.layer[L].lv_vs_lu[s] = new TH2D(hname, htitle, nbins, xmin, xmax, nbins, xmin, xmax);
        }
      }
    }
  }

  void RGAFiducialFilterValidator::Start(hipo::banklist& banks)
  {
    // Build sequence: run fiducial filter, then plot surviving tracks
    m_algo_seq = std::make_unique<AlgorithmSequence>();
    m_algo_seq->Add("clas12::RGAFiducialFilter");

    // Optional runtime override for strictness (no YAML default in algorithm):
    // IGUANA_RGAFID_STRICTNESS in {1,2,3}
    if (const char* env = std::getenv("IGUANA_RGAFID_STRICTNESS")) {
      try {
        int s = std::stoi(env);
        s = std::max(1, std::min(3, s));
        u_strictness_override = s;
      } catch (...) { /* keep default 1 */ }
    }
    m_log->Info("RGAFiducialFilterValidator: strictness override = {}", u_strictness_override);
    // This SetOption is informational; the algorithm itself reads the env var.
    m_algo_seq->SetOption<std::vector<int>>("clas12::RGAFiducialFilter", "strictness", { u_strictness_override });

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

    // book plots
    for (auto pid : u_pid_list) BookPlotsForPID(pid);
  }

  void RGAFiducialFilterValidator::Run(hipo::banklist& banks) const
  {
    auto& particle_bank = GetBank(banks, b_particle, "REC::Particle");
    auto& calor_bank    = GetBank(banks, b_calor,    "REC::Calorimeter");

    // Run the filter first; plot hits associated with surviving tracks only
    m_algo_seq->Run(banks);

    // survivor sets keyed by pid
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

    // Fill 2D hists
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

      for (auto pid : u_pid_list) {
        auto it = survivors.find(pid);
        if (it == survivors.end()) continue;
        if (it->second.find(pindex) == it->second.end()) continue;

        auto& grid = const_cast<RGAFiducialFilterValidator*>(this)->u_plots[pid];
        if (auto* h = grid.layer[L].lv_vs_lw[sector]) h->Fill(lv, lw);
        if (auto* h = grid.layer[L].lv_vs_lu[sector]) h->Fill(lv, lu);
      }
    }
  }

  // Draw a 2×3 sector grid for either lv_vs_lw (lv_vs_lw=true) or lv_vs_lu (false)
  void RGAFiducialFilterValidator::DrawSectorGrid2D(int pid, int layer_idx, bool lv_vs_lw)
  {
    auto& layer = u_plots.at(pid).layer[layer_idx];

    gStyle->SetOptStat(0);

    TString canv_name = Form("rgafid2D_pid%d_%s_%s",
                             pid, LayerName(layer_idx), lv_vs_lw ? "lv_lw" : "lv_lu");
    TString canv_title = Form("%s — %s — %s",
                              PIDName(pid), LayerName(layer_idx), lv_vs_lw ? "lv vs lw" : "lv vs lu");

    auto canv = new TCanvas(canv_name, canv_title, 1600, 1000);
    canv->Divide(3, 2); // 2 rows × 3 cols = 6 sectors

    for (int s = 1; s <= 6; ++s) {
      canv->cd(s);
      TH2D* h = lv_vs_lw ? layer.lv_vs_lw[s] : layer.lv_vs_lu[s];
      if (!h) continue;
      // Set sector-specific title for clarity; axis titles already set on booking
      TString title = Form("%s — %s — Sector %d", PIDName(pid), LayerName(layer_idx), s);
      h->SetTitle(title);
      h->Draw("COLZ");
    }

    TString png = Form("%s_pid%d_%s_%s.png",
                       m_output_file_basename.Data(), pid, LayerName(layer_idx), lv_vs_lw ? "lv_lw" : "lv_lu");
    canv->SaveAs(png);
  }

  void RGAFiducialFilterValidator::Stop()
  {
    if (GetOutputDirectory()) {
      for (auto pid : u_pid_list) {
        for (int L = 0; L < 3; ++L) {
          DrawSectorGrid2D(pid, L, /*lv_vs_lw=*/true);
          DrawSectorGrid2D(pid, L, /*lv_vs_lu=*/false);
        }
      }
      if (m_output_file) {
        m_output_file->Write();
        m_log->Info("Wrote output file {}", m_output_file->GetName());
        m_output_file->Close();
      }
    }
  }

} // namespace iguana::clas12