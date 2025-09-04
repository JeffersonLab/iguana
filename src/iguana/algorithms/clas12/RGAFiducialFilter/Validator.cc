#include "Validator.h"
#include <cstdlib>   // setenv / _putenv_s

#include <TCanvas.h>
#include <TPad.h>
#include <TLegend.h>
#include <unordered_set>

namespace iguana::clas12 {

  REGISTER_IGUANA_VALIDATOR(RGAFiducialFilterValidator);

  static bool banklist_has(hipo::banklist& banks, const char* name) {
    for (auto& b : banks) {
      if (b.getSchema().getName() == name) return true;
    }
    return false;
  }

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

    // 90 bins x 4.5 cm = 405 cm range (calorimeter axes)
    const int    nb = 90;
    const double lo = 0.0;
    const double hi = 405.0;

    for (int L = 0; L < 3; ++L) {
      for (int s = 1; s <= 6; ++s) {
        if (!sets.layer[L].lv_lw.sec[s]) {
          TString hname  = Form("h2_%d_%s_lv_lw_s%d", pid, LayerName(L), s);
          TString htitle = Form("%s %s Sector %d;lv (cm);lw (cm)", PIDName(pid), LayerName(L), s);
          auto* h = new TH2D(hname, htitle, nb, lo, hi, nb, lo, hi);
          h->SetStats(0); h->GetXaxis()->SetTitleOffset(1.1); h->GetYaxis()->SetTitleOffset(1.25);
          sets.layer[L].lv_lw.sec[s] = h;
        }
        if (!sets.layer[L].lv_lu.sec[s]) {
          TString hname  = Form("h2_%d_%s_lv_lu_s%d", pid, LayerName(L), s);
          TString htitle = Form("%s %s Sector %d;lv (cm);lu (cm)", PIDName(pid), LayerName(L), s);
          auto* h = new TH2D(hname, htitle, nb, lo, hi, nb, lo, hi);
          h->SetStats(0); h->GetXaxis()->SetTitleOffset(1.1); h->GetYaxis()->SetTitleOffset(1.25);
          sets.layer[L].lv_lu.sec[s] = h;
        }
      }
    }

    // FT per-PID occupancy (y vs x) — single canvas later (post-cuts only)
    if (!u_ft_xy[pid]) {
      TString hname  = Form("h2_ft_xy_pid%d", pid);
      TString htitle = Form("Forward Tagger - %s;y (cm);x (cm)", PIDName(pid));
      u_ft_xy[pid] = new TH2D(hname, htitle, 200, -20.0, 20.0, 200, -20.0, 20.0);
      u_ft_xy[pid]->SetStats(0);
      u_ft_xy[pid]->GetXaxis()->SetTitleOffset(1.1);
      u_ft_xy[pid]->GetYaxis()->SetTitleOffset(1.25);
    }
  }

  void RGAFiducialFilterValidator::Start(hipo::banklist& banks)
  {
    // Force strictness=3 in validator so we exercise dead-PMT masks
    #if defined(_WIN32)
      _putenv_s("IGUANA_RGAFID_STRICTNESS", "3");
    #else
      setenv("IGUANA_RGAFID_STRICTNESS", "3", 1);
    #endif

    // Also default the debug to on for one pass (user can turn off via env)
    if (!std::getenv("IGUANA_RGAFID_DEBUG"))
      setenv("IGUANA_RGAFID_DEBUG", "1", 0);
    if (!std::getenv("IGUANA_RGAFID_DEBUG_EVENTS"))
      setenv("IGUANA_RGAFID_DEBUG_EVENTS", "50", 0); // print first 50 track decisions

    m_algo_seq = std::make_unique<AlgorithmSequence>();
    m_algo_seq->Add("clas12::RGAFiducialFilter");
    m_algo_seq->Start(banks);

    // required
    b_particle = GetBankIndex(banks, "REC::Particle");

    // optional banks
    if (banklist_has(banks, "REC::Calorimeter")) {
      b_calor = GetBankIndex(banks, "REC::Calorimeter"); m_have_calor = true;
    } else {
      m_have_calor = false;
      m_log->Info("Optional bank 'REC::Calorimeter' not in banklist; calorimeter plots will be skipped.");
    }

    if (banklist_has(banks, "REC::ForwardTagger")) {
      b_ft = GetBankIndex(banks, "REC::ForwardTagger"); m_have_ft = true;
    } else {
      m_have_ft = false;
      m_log->Info("Optional bank 'REC::ForwardTagger' not in banklist; FT plots will be skipped.");
    }

    // output base: rga_fiducial
    if (auto output_dir = GetOutputDirectory()) {
      m_output_file_basename = output_dir.value() + std::string("/rga_fiducial");
      m_output_file          = new TFile(m_output_file_basename + ".root", "RECREATE");
    }

    // book plots
    for (auto pid : u_pid_list) BookPlotsForPID(pid);
  }

  void RGAFiducialFilterValidator::Run(hipo::banklist& banks) const
  {
    auto& particle_bank = GetBank(banks, b_particle, "REC::Particle");

    // Count before
    int before = static_cast<int>(particle_bank.getRowList().size());

    // run the filter first (bank is filtered in-place)
    m_algo_seq->Run(banks);

    // survivors by pid (post-cut)
    std::unordered_map<int, std::unordered_set<int>> survivors;
    for (auto pid : u_pid_list) survivors[pid];

    for (auto const& row : particle_bank.getRowList()) {
      const int pid = particle_bank.getInt("pid", row);
      if (survivors.find(pid) != survivors.end()) survivors[pid].insert(static_cast<int>(row));
    }

    int after = static_cast<int>(particle_bank.getRowList().size());
    m_log->Info("[VALIDATOR] REC::Particle rows: before={} after={} kept%={:.1f}",
                before, after, before>0 ? 100.0*after/before : 0.0);

    // Calorimeter fills (survivors only)
    if (m_have_calor) {
      auto& calor_bank = GetBank(banks, b_calor, "REC::Calorimeter");
      const int ncal = calor_bank.getRows();
      for (int i = 0; i < ncal; ++i) {
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

          auto& sets = const_cast<RGAFiducialFilterValidator*>(this)->u_plots2d[pid];
          sets.layer[L].lv_lw.sec[sector]->Fill(lv, lw);
          sets.layer[L].lv_lu.sec[sector]->Fill(lv, lu);
        }
      }
    }

    // FT fills (survivors only)
    if (m_have_ft) {
      auto& ft_bank = GetBank(banks, b_ft, "REC::ForwardTagger");
      const int nft = ft_bank.getRows();
      for (int i = 0; i < nft; ++i) {
        const int pindex = ft_bank.getInt("pindex", i);
        const float x    = ft_bank.getFloat("x", i);
        const float y    = ft_bank.getFloat("y", i);

        for (auto pid : u_pid_list) {
          auto it = survivors.find(pid);
          if (it == survivors.end()) continue;
          if (it->second.find(pindex) == it->second.end()) continue;
          auto* h = const_cast<RGAFiducialFilterValidator*>(this)->u_ft_xy[pid];
          if (h) h->Fill(y, x);
        }
      }
    }
  }

  void RGAFiducialFilterValidator::DrawSectorGrid2D(int pid, int layer_idx, bool lv_vs_lw)
  {
    auto& sets = u_plots2d.at(pid);
    const char* proj = lv_vs_lw ? "lv_vs_lw" : "lv_vs_lu";

    auto* canv = new TCanvas(Form("rgafid_%s_%s_pid%d", LayerName(layer_idx), proj, pid),
                             Form("%s %s - %s", PIDName(pid), LayerName(layer_idx),
                                  lv_vs_lw ? "lv vs lw" : "lv vs lu"),
                             1400, 900);
    canv->Divide(3, 2);

    for (int s = 1; s <= 6; ++s) {
      canv->cd(s);
      gPad->SetLeftMargin(0.16); gPad->SetRightMargin(0.14);
      gPad->SetBottomMargin(0.12); gPad->SetTopMargin(0.08);

      TH2D* h = lv_vs_lw ? sets.layer[layer_idx].lv_lw.sec[s]
                         : sets.layer[layer_idx].lv_lu.sec[s];
      if (!h) continue;
      h->SetTitle(Form("%s %s Sector %d", PIDName(pid), LayerName(layer_idx), s));
      h->Draw("COLZ");
    }

    TString png_name = Form("%s_calorimeter_%s_%s_pid%d.png",
                            m_output_file_basename.Data(),
                            LayerName(layer_idx), proj, pid);
    canv->SaveAs(png_name);
  }

  void RGAFiducialFilterValidator::DrawFTCanvas()
  {
    // Single FT canvas with cuts enforced (we filled after running the filter)
    auto* canv = new TCanvas("rgafid_ft_xy", "Forward Tagger - y vs x (post-cuts)", 1200, 600);
    canv->Divide(2, 1);

    int pad = 1;
    for (auto pid : u_pid_list) {
      canv->cd(pad++);
      gPad->SetLeftMargin(0.16); gPad->SetRightMargin(0.14);
      gPad->SetBottomMargin(0.12); gPad->SetTopMargin(0.08);

      TH2D* h = u_ft_xy[pid];
      if (!h) continue;
      h->Draw("COLZ");
    }

    TString png_name = m_output_file_basename + "_ft_xy.png";
    canv->SaveAs(png_name);
  }

  void RGAFiducialFilterValidator::Stop()
  {
    if (!GetOutputDirectory()) return;

    if (m_have_calor) {
      for (auto pid : u_pid_list)
        for (int L = 0; L < 3; ++L) {
          DrawSectorGrid2D(pid, L, true );
          DrawSectorGrid2D(pid, L, false);
        }
    }
    if (m_have_ft) DrawFTCanvas();

    if (m_output_file) {
      m_output_file->Write();
      m_log->Info("Wrote output file {}", m_output_file->GetName());
      m_output_file->Close();
    }
  }

} // namespace iguana::clas12