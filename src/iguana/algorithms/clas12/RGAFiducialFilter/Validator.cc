// src/iguana/algorithms/clas12/RGAFiducialFilter/Validator.cc

#include "Validator.h"

#include <TCanvas.h>
#include <TLegend.h>
#include <TEllipse.h>
#include <TStyle.h>
#include <TString.h>

#include <set>
#include <unordered_map>
#include <cmath>

namespace iguana::clas12 {

REGISTER_IGUANA_VALIDATOR(RGAFiducialFilterValidator);

// small util
static bool banklist_has(hipo::banklist& banks, const char* name) {
  for (auto& b : banks) if (b.getSchema().getName() == name) return true;
  return false;
}

// ----------------------------------------------------------------------------
// No YAML reads here - hard-coded FT overlay defaults.
// ----------------------------------------------------------------------------
void RGAFiducialFilterValidator::LoadFTParamsFromYAML()
{
  // Defaults: annulus [8.5, 15.5] cm and 4 holes
  m_ftdraw.rmin = 8.5f;
  m_ftdraw.rmax = 15.5f;
  m_ftdraw.holes.clear();
  m_ftdraw.holes.push_back({1.60f, -8.42f,  9.89f});
  m_ftdraw.holes.push_back({1.60f, -9.89f, -5.33f});
  m_ftdraw.holes.push_back({2.30f, -6.15f, -13.00f});
  m_ftdraw.holes.push_back({2.00f,  3.70f,  -6.50f});
}

void RGAFiducialFilterValidator::BookIfNeeded()
{
  // PCAL: range 0..27 cm, 0.5 cm bins
  const int nb=54; const double lo=0.0, hi=27.0;

  for (int pid : kPIDs) {
    auto& P = m_cal[pid];
    for (int s=1; s<=6; ++s) {
      if (!P[s].lv_kept) {
        P[s].lv_kept = new TH1D(Form("h_pcal_lv_kept_pid%d_s%d", pid, s),
                                Form("PID %d S%d;length (cm);counts", pid, s), nb, lo, hi);
        P[s].lv_kept->SetStats(0);
      }
      if (!P[s].lv_cut) {
        P[s].lv_cut = new TH1D(Form("h_pcal_lv_cut_pid%d_s%d", pid, s),
                               Form("PID %d S%d;length (cm);counts", pid, s), nb, lo, hi);
        P[s].lv_cut->SetStats(0);
      }
      if (!P[s].lw_kept) {
        P[s].lw_kept = new TH1D(Form("h_pcal_lw_kept_pid%d_s%d", pid, s),
                                Form("PID %d S%d;length (cm);counts", pid, s), nb, lo, hi);
        P[s].lw_kept->SetStats(0);
      }
      if (!P[s].lw_cut) {
        P[s].lw_cut = new TH1D(Form("h_pcal_lw_cut_pid%d_s%d", pid, s),
                               Form("PID %d S%d;length (cm);counts", pid, s), nb, lo, hi);
        P[s].lw_cut->SetStats(0);
      }
    }
  }

  // FT: generous range for x,y
  for (int pid : kPIDs) {
    auto& F = m_ft_h[pid];
    if (!F.before)
      F.before = new TH2F(Form("h_ft_before_pid%d", pid),
                          Form("FT x-y before (PID %d);x (cm);y (cm)", pid),
                          120, -30, 30, 120, -30, 30);
    if (!F.after)
      F.after  = new TH2F(Form("h_ft_after_pid%d", pid),
                          Form("FT x-y after (PID %d);x (cm);y (cm)", pid),
                          120, -30, 30, 120, -30, 30);
    F.before->SetStats(0);
    F.after->SetStats(0);
  }

  // CVT layer 12: phi (x) vs theta (y)
  for (int pid : kPIDs) {
    auto& C = m_cvt_h[pid];
    if (!C.before)
      C.before = new TH2F(Form("h_cvt_l12_phi_theta_before_pid%d", pid),
                          Form("CVT layer 12 before (PID %d);phi (deg);theta (deg)", pid),
                          180, 0, 360,  90, 0, 90);
    if (!C.after)
      C.after  = new TH2F(Form("h_cvt_l12_phi_theta_after_pid%d", pid),
                          Form("CVT layer 12 after (PID %d);phi (deg);theta (deg)", pid),
                          180, 0, 360,  90, 0, 90);
    C.before->SetStats(0);
    C.after->SetStats(0);
  }
}

void RGAFiducialFilterValidator::Start(hipo::banklist& banks)
{
  // Build/Start sequence
  m_seq = std::make_unique<AlgorithmSequence>();
  m_seq->Add("clas12::RGAFiducialFilter");
  m_seq->Start(banks);

  // Banks
  b_particle = GetBankIndex(banks, "REC::Particle");
  if (banklist_has(banks, "REC::Calorimeter")) {
    b_calor = GetBankIndex(banks, "REC::Calorimeter"); m_have_calor=true;
  }
  if (banklist_has(banks, "REC::ForwardTagger")) {
    b_ft = GetBankIndex(banks, "REC::ForwardTagger"); m_have_ft=true;
  }
  // REC::Traj for CVT plotting
  if (banklist_has(banks, "REC::Traj")) {
    b_traj = GetBankIndex(banks, "REC::Traj"); m_have_traj=true;
  } else {
    m_have_traj=false;
    m_log->Info("[RGAFID][VAL] REC::Traj not provided; CVT plots disabled. "
                "Re-run with -b REC::Traj to enable CVT before/after plots.");
  }

  // FT overlay params (defaults only; no YAML here to avoid crashes)
  LoadFTParamsFromYAML();

  // Output
  if (auto dir = GetOutputDirectory()) {
    m_base.Form("%s/rga_fiducial", dir->c_str());
    m_out  = new TFile(Form("%s.root", m_base.Data()), "RECREATE");
  } else {
    m_base = "rga_fiducial";
    m_out  = nullptr;
  }

  BookIfNeeded();
}

void RGAFiducialFilterValidator::Run(hipo::banklist& banks) const
{
  auto& particle = GetBank(banks, b_particle, "REC::Particle");

  // --- BEFORE snapshot (pindex -> pid for 11/22)
  std::unordered_map<int,int> before_pid;
  for (auto const& row : particle.getRowList()) {
    int pid = particle.getInt("pid", row);
    if (pid==11 || pid==22) before_pid.emplace((int)row, pid);
  }
  if (before_pid.empty()) return;

  // Run the filter (alters REC::Particle)
  m_seq->Run(banks);

  // --- AFTER snapshot
  std::unordered_map<int,int> after_pid;
  for (auto const& row : particle.getRowList()) {
    int pid = particle.getInt("pid", row);
    if (pid==11 || pid==22) after_pid.emplace((int)row, pid);
  }

  // --- Fill PCAL lv/lw kept vs cut
  if (m_have_calor) {
    auto& cal = GetBank(banks, b_calor, "REC::Calorimeter");

    // Determine which pindices are cut: present in before but not in after
    auto is_kept = [&](int pidx){ return after_pid.find(pidx) != after_pid.end(); };

    const int n = cal.getRows();
    for (int i=0;i<n;++i) {
      int pidx = cal.getInt("pindex", i);
      auto itb = before_pid.find(pidx);
      if (itb == before_pid.end()) continue;         // not an e-/gamma before
      if (cal.getInt("layer", i) != 1) continue;     // PCAL only

      int pid = itb->second;
      int sec = cal.getInt("sector", i);
      if (sec < 1 || sec > 6) continue;

      double lv = cal.getFloat("lv", i);
      double lw = cal.getFloat("lw", i);
      bool kept = is_kept(pidx);

      auto& H = const_cast<RGAFiducialFilterValidator*>(this)->m_cal[pid][sec];
      if (lv >= 0.0 && lv <= 27.0) (kept ? H.lv_kept : H.lv_cut)->Fill(lv);
      if (lw >= 0.0 && lw <= 27.0) (kept ? H.lw_kept : H.lw_cut)->Fill(lw);
    }
  }

  // --- Fill FT before/after x-y
  if (m_have_ft) {
    auto& ft = GetBank(banks, b_ft, "REC::ForwardTagger");

    std::set<int> seen_before, seen_after; // avoid double counting if >1 row/pindex

    const int n = ft.getRows();
    for (int i=0;i<n;++i) {
      int pidx = ft.getInt("pindex", i);

      auto itb = before_pid.find(pidx);
      if (itb != before_pid.end() && !seen_before.count(pidx)) {
        m_ft_h.at(itb->second).before->Fill(ft.getFloat("x", i), ft.getFloat("y", i));
        seen_before.insert(pidx);
      }

      auto ita = after_pid.find(pidx);
      if (ita != after_pid.end() && !seen_after.count(pidx)) {
        m_ft_h.at(ita->second).after->Fill(ft.getFloat("x", i), ft.getFloat("y", i));
        seen_after.insert(pidx);
      }
    }
  }

  // --- Fill CVT L12 before/after (phi vs theta) using REC::Traj detector==5
  if (m_have_traj) {
    auto& traj = GetBank(banks, b_traj, "REC::Traj");

    std::set<int> b_seen, a_seen;

    const int n = traj.getRows();
    for (int i=0; i<n; ++i) {
      if (traj.getInt("detector", i) != 5) continue; // CVT only

      int pidx  = traj.getInt("pindex", i);
      int layer = traj.getInt("layer", i);
      if (layer != 12) continue;

      double x = traj.getFloat("x", i);
      double y = traj.getFloat("y", i);
      double z = traj.getFloat("z", i);

      // angles in degrees
      double phi = std::atan2(y, x) * (180.0/M_PI); if (phi < 0) phi += 360.0;
      double rho = std::sqrt(x*x + y*y);
      double theta = std::atan2(rho, (z==0.0 ? 1e-9 : z)) * (180.0/M_PI);

      auto itb = before_pid.find(pidx);
      if (itb != before_pid.end() && !b_seen.count(pidx)) {
        m_cvt_h.at(itb->second).before->Fill(phi, theta);
        b_seen.insert(pidx);
      }
      auto ita = after_pid.find(pidx);
      if (ita != after_pid.end() && !a_seen.count(pidx)) {
        m_cvt_h.at(ita->second).after->Fill(phi, theta);
        a_seen.insert(pidx);
      }
    }
  }
}

void RGAFiducialFilterValidator::DrawCalCanvas(int pid, const char* title)
{
  auto it = m_cal.find(pid);
  if (it == m_cal.end()) return;

  auto* c = new TCanvas(Form("rgafid_pcal_pid%d", pid), title, 1400, 900);
  c->Divide(3,2);

  for (int s=1; s<=6; ++s) {
    c->cd(s);
    gPad->SetLeftMargin(0.12); gPad->SetRightMargin(0.04);
    gPad->SetBottomMargin(0.12); gPad->SetTopMargin(0.08);

    auto& H = it->second[s];
    if (!H.lv_kept) continue;

    // styles: kept = solid, cut = dashed
    H.lv_kept->SetLineColor(kBlue+1);  H.lv_kept->SetLineWidth(2);  H.lv_kept->SetLineStyle(1);
    H.lw_kept->SetLineColor(kRed+1);   H.lw_kept->SetLineWidth(2);  H.lw_kept->SetLineStyle(1);
    H.lv_cut ->SetLineColor(kBlue+1);  H.lv_cut ->SetLineWidth(2);  H.lv_cut ->SetLineStyle(2);
    H.lw_cut ->SetLineColor(kRed+1);   H.lw_cut ->SetLineWidth(2);  H.lw_cut ->SetLineStyle(2);

    // explicit title on lv kept (pad title)
    H.lv_kept->SetTitle(Form("%s - Sector %d;length (cm);counts",
                      pid==11?"Electrons":"Photons", s));

    // draw in order so axes come from a "kept" spectrum
    H.lv_kept->Draw("HIST");
    H.lw_kept->Draw("HISTSAME");
    H.lv_cut ->Draw("HISTSAME");
    H.lw_cut ->Draw("HISTSAME");

    auto* leg = new TLegend(0.55, 0.72, 0.88, 0.90);
    leg->SetBorderSize(0); leg->SetFillStyle(0);
    leg->AddEntry(H.lv_kept, "lv kept", "l");
    leg->AddEntry(H.lw_kept, "lw kept", "l");
    leg->AddEntry(H.lv_cut , "lv cut",  "l");
    leg->AddEntry(H.lw_cut , "lw cut",  "l");
    leg->Draw();
  }

  c->SaveAs(Form("%s_pcal_lv_lw_pid%d.png", m_base.Data(), pid));
}

void RGAFiducialFilterValidator::DrawFTCanvas2x2()
{
  if (!m_have_ft) return;

  auto* c = new TCanvas("rgafid_ft_xy_2x2", "FT x-y Before/After", 1200, 900);
  c->Divide(2,2);

  auto draw_pad = [&](int pad, TH2F* h, const char* ttl){
    c->cd(pad);
    gPad->SetLeftMargin(0.12); gPad->SetRightMargin(0.04);
    gPad->SetBottomMargin(0.12); gPad->SetTopMargin(0.08);
    h->SetTitle(ttl);
    h->Draw("COLZ");

    // overlays: annulus + holes (no legend text)
    auto* outer = new TEllipse(0,0, m_ftdraw.rmax, m_ftdraw.rmax);
    auto* inner = new TEllipse(0,0, m_ftdraw.rmin, m_ftdraw.rmin);
    outer->SetFillStyle(0); inner->SetFillStyle(0);
    outer->SetLineStyle(2); inner->SetLineStyle(2);
    outer->Draw(); inner->Draw();
    for (auto const& H : m_ftdraw.holes) {
      auto* e = new TEllipse(H[1], H[2], H[0], H[0]);
      e->SetFillStyle(0); e->SetLineColor(kBlack); e->SetLineStyle(7); e->Draw();
    }
  };

  // electrons row
  draw_pad(1, m_ft_h.at(11).before, "Electrons (before cuts);x (cm);y (cm)");
  draw_pad(2, m_ft_h.at(11).after,  "Electrons (after cuts);x (cm);y (cm)");

  // photons row
  draw_pad(3, m_ft_h.at(22).before, "Photons (before cuts);x (cm);y (cm)");
  draw_pad(4, m_ft_h.at(22).after,  "Photons (after cuts);x (cm);y (cm)");

  c->SaveAs(Form("%s_ft_xy_2x2.png", m_base.Data()));
}

void RGAFiducialFilterValidator::DrawCVTCanvas1x2(int pid, const char* title)
{
  if (!m_have_traj) return;

  auto it = m_cvt_h.find(pid);
  if (it == m_cvt_h.end() || !it->second.before || !it->second.after) return;

  auto* c = new TCanvas(Form("rgafid_cvt_l12_pid%d", pid), title, 1200, 600);
  c->Divide(2,1);

  c->cd(1);
  gPad->SetLeftMargin(0.12); gPad->SetRightMargin(0.08);
  gPad->SetBottomMargin(0.12); gPad->SetTopMargin(0.08);
  it->second.before->Draw("COLZ");

  c->cd(2);
  gPad->SetLeftMargin(0.12); gPad->SetRightMargin(0.08);
  gPad->SetBottomMargin(0.12); gPad->SetTopMargin(0.08);
  it->second.after->Draw("COLZ");

  c->SaveAs(Form("%s_cvt_l12_phi_theta_pid%d.png", m_base.Data(), pid));
}

void RGAFiducialFilterValidator::Stop()
{
  // Draw PCAL canvases
  DrawCalCanvas(11, "PCAL lv & lw (Electrons): kept solid, cut dashed");
  DrawCalCanvas(22, "PCAL lv & lw (Photons): kept solid, cut dashed");

  // Draw FT 2x2 (no explanatory legend text)
  DrawFTCanvas2x2();

  // Draw CVT L12 1x2 (phi vs theta)
  DrawCVTCanvas1x2(11, "CVT layer 12 (Electrons): phi vs theta");
  DrawCVTCanvas1x2(22, "CVT layer 12 (Photons): phi vs theta");

  if (m_out) {
    m_out->Write();
    m_log->Info("Wrote output file {}", m_out->GetName());
    m_out->Close();
  }
}

} // namespace iguana::clas12