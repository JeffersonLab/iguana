// src/iguana/algorithms/clas12/RGAFiducialFilter/Validator.cc

#include "Validator.h"

#include <TCanvas.h>
#include <TLegend.h>
#include <TEllipse.h>
#include <TStyle.h>
#include <TString.h>

#include <set>
#include <unordered_map>
#include <unordered_set>
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

  // CVT layer 12: combined hadrons, phi (x) vs theta (y)
  if (!m_cvt_before)
    m_cvt_before = new TH2F("h_cvt_l12_phi_theta_before_all",
                            "CVT layer 12 before (hadrons: #pm211,#pm321,#pm2212);phi (deg);theta (deg)",
                            180, 0, 360,  90, 0, 90);
  if (!m_cvt_after)
    m_cvt_after  = new TH2F("h_cvt_l12_phi_theta_after_all",
                            "CVT layer 12 after (hadrons: #pm211,#pm321,#pm2212);phi (deg);theta (deg)",
                            180, 0, 360,  90, 0, 90);
  m_cvt_before->SetStats(0);
  m_cvt_after->SetStats(0);

  // DC edge distributions (range 0..30 cm, 0.3 cm bins)
  auto mk = [](const char* name, const char* title){
    auto* h = new TH1D(name, title, 100, 0.0, 30.0);
    h->SetStats(0);
    return h;
  };
  if (!m_dc_pos.r1_before) {
    m_dc_pos.r1_before = mk("h_dc_pos_r1_before", "DC R1 before (+);edge (cm);counts");
    m_dc_pos.r2_before = mk("h_dc_pos_r2_before", "DC R2 before (+);edge (cm);counts");
    m_dc_pos.r3_before = mk("h_dc_pos_r3_before", "DC R3 before (+);edge (cm);counts");
    m_dc_pos.r1_after  = mk("h_dc_pos_r1_after",  "DC R1 after (+);edge (cm);counts");
    m_dc_pos.r2_after  = mk("h_dc_pos_r2_after",  "DC R2 after (+);edge (cm);counts");
    m_dc_pos.r3_after  = mk("h_dc_pos_r3_after",  "DC R3 after (+);edge (cm);counts");
  }
  if (!m_dc_neg.r1_before) {
    m_dc_neg.r1_before = mk("h_dc_neg_r1_before", "DC R1 before (-);edge (cm);counts");
    m_dc_neg.r2_before = mk("h_dc_neg_r2_before", "DC R2 before (-);edge (cm);counts");
    m_dc_neg.r3_before = mk("h_dc_neg_r3_before", "DC R3 before (-);edge (cm);counts");
    m_dc_neg.r1_after  = mk("h_dc_neg_r1_after",  "DC R1 after (-);edge (cm);counts");
    m_dc_neg.r2_after  = mk("h_dc_neg_r2_after",  "DC R2 after (-);edge (cm);counts");
    m_dc_neg.r3_after  = mk("h_dc_neg_r3_after",  "DC R3 after (-);edge (cm);counts");
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
  // REC::Traj for CVT/DC plotting
  if (banklist_has(banks, "REC::Traj")) {
    b_traj = GetBankIndex(banks, "REC::Traj"); m_have_traj=true;
  } else {
    m_have_traj=false;
    m_log->Info("[RGAFID][VAL] REC::Traj not provided; CVT/DC plots disabled. "
                "Re-run with -b REC::Traj to enable trajectory-based plots.");
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

  // --- BEFORE snapshot: electrons/photons for PCAL/FT
  std::unordered_map<int,int> before_pid;
  for (auto const& row : particle.getRowList()) {
    int pid = particle.getInt("pid", row);
    if (pid==11 || pid==22) before_pid.emplace((int)row, pid);
  }

  // --- BEFORE snapshot: hadrons for CVT (combined)
  auto is_hadron = [](int pid){
    return pid==211 || pid==321 || pid==2212 ||
           pid==-211 || pid==-321 || pid==-2212;
  };
  std::unordered_set<int> cvt_before;
  for (auto const& row : particle.getRowList()) {
    int pid = particle.getInt("pid", row);
    if (is_hadron(pid)) cvt_before.insert((int)row);
  }

  // --- BEFORE snapshot: DC-eligible pindices by sign (require REC::Traj detector==6 present)
  std::unordered_set<int> dc_pos_before, dc_neg_before;
  if (m_have_traj) {
    auto& traj = GetBank(banks, b_traj, "REC::Traj");
    std::unordered_set<int> has_dc;
    const int n = traj.getRows();
    for (int i=0;i<n;++i) if (traj.getInt("detector", i) == 6) has_dc.insert(traj.getInt("pindex", i));

    for (auto const& row : particle.getRowList()) {
      int pidx = (int)row;
      if (!has_dc.count(pidx)) continue;

      int pid = particle.getInt("pid", row);
      if (pid == 22 || pid == 0) continue; // neutrals ignored
      if (pid > 0) dc_pos_before.insert(pidx);
      else         dc_neg_before.insert(pidx);
    }
  }

  // Run the filter (alters REC::Particle)
  m_seq->Run(banks);

  // --- AFTER snapshot: electrons/photons
  std::unordered_map<int,int> after_pid;
  for (auto const& row : particle.getRowList()) {
    int pid = particle.getInt("pid", row);
    if (pid==11 || pid==22) after_pid.emplace((int)row, pid);
  }

  // --- AFTER snapshot: hadrons for CVT
  std::unordered_set<int> cvt_after;
  for (auto const& row : particle.getRowList()) {
    int pid = particle.getInt("pid", row);
    if (is_hadron(pid)) cvt_after.insert((int)row);
  }

  // --- AFTER snapshot: DC by sign (intersection with survivors)
  std::unordered_set<int> dc_pos_after, dc_neg_after;
  for (auto const& row : particle.getRowList()) {
    int pidx = (int)row;
    if (dc_pos_before.count(pidx)) dc_pos_after.insert(pidx);
    if (dc_neg_before.count(pidx)) dc_neg_after.insert(pidx);
  }

  // --- Fill PCAL lv/lw kept vs cut (uses e/gamma only)
  if (m_have_calor) {
    auto& cal = GetBank(banks, b_calor, "REC::Calorimeter");
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

  // --- Fill FT before/after x-y (uses e/gamma only)
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

  // --- Fill CVT L12 before/after (phi vs theta) using REC::Traj detector==5, for hadrons
  if (m_have_traj) {
    auto& traj = GetBank(banks, b_traj, "REC::Traj");

    std::set<int> b_seen, a_seen;

    const int n = traj.getRows();
    for (int i=0; i<n; ++i) {
      if (traj.getInt("detector", i) != 5) continue; // CVT only
      if (traj.getInt("layer", i) != 12) continue;   // layer 12 only

      int pidx = traj.getInt("pindex", i);

      double x = traj.getFloat("x", i);
      double y = traj.getFloat("y", i);
      double z = traj.getFloat("z", i);

      // angles in degrees
      double phi = std::atan2(y, x) * (180.0/M_PI); if (phi < 0) phi += 360.0;
      double rho = std::sqrt(x*x + y*y);
      double theta = std::atan2(rho, (z==0.0 ? 1e-9 : z)) * (180.0/M_PI);

      if (cvt_before.count(pidx) && !b_seen.count(pidx)) {
        m_cvt_before->Fill(phi, theta);
        b_seen.insert(pidx);
      }
      if (cvt_after.count(pidx) && !a_seen.count(pidx)) {
        m_cvt_after->Fill(phi, theta);
        a_seen.insert(pidx);
      }
    }
    // accumulate counts (one per unique pindex per event)
    const_cast<RGAFiducialFilterValidator*>(this)->m_cvt_before_n += (long long) (cvt_before.size());
    const_cast<RGAFiducialFilterValidator*>(this)->m_cvt_after_n  += (long long) (cvt_after.size());
  }

  // --- Fill DC edges (detector==6), regions 1/2/3 (layers 6/18/36), separate pos/neg
  if (m_have_traj) {
    auto& traj = GetBank(banks, b_traj, "REC::Traj");

    // Count survivors (per-event unique pindex by sign)
    const_cast<RGAFiducialFilterValidator*>(this)->m_dc_pos_before_n += (long long) dc_pos_before.size();
    const_cast<RGAFiducialFilterValidator*>(this)->m_dc_pos_after_n  += (long long) dc_pos_after.size();
    const_cast<RGAFiducialFilterValidator*>(this)->m_dc_neg_before_n += (long long) dc_neg_before.size();
    const_cast<RGAFiducialFilterValidator*>(this)->m_dc_neg_after_n  += (long long) dc_neg_after.size();

    // Avoid double filling per region
    std::set<int> pos_seen_r1_b, pos_seen_r2_b, pos_seen_r3_b;
    std::set<int> pos_seen_r1_a, pos_seen_r2_a, pos_seen_r3_a;
    std::set<int> neg_seen_r1_b, neg_seen_r2_b, neg_seen_r3_b;
    std::set<int> neg_seen_r1_a, neg_seen_r2_a, neg_seen_r3_a;

    const int n = traj.getRows();
    for (int i=0;i<n;++i) {
      if (traj.getInt("detector", i) != 6) continue;

      const int layer = traj.getInt("layer", i);
      int pidx = traj.getInt("pindex", i);
      double edge = traj.getFloat("edge", i);

      // map layer -> region index & hist pointers
      auto fill_dc = [&](bool positive, bool before){
        if (layer == 6) {
          if (positive) {
            if (before && !pos_seen_r1_b.count(pidx)) { m_dc_pos.r1_before->Fill(edge); pos_seen_r1_b.insert(pidx); }
            if (!before && !pos_seen_r1_a.count(pidx) && (dc_pos_after.count(pidx))) { m_dc_pos.r1_after->Fill(edge); pos_seen_r1_a.insert(pidx); }
          } else {
            if (before && !neg_seen_r1_b.count(pidx)) { m_dc_neg.r1_before->Fill(edge); neg_seen_r1_b.insert(pidx); }
            if (!before && !neg_seen_r1_a.count(pidx) && (dc_neg_after.count(pidx))) { m_dc_neg.r1_after->Fill(edge); neg_seen_r1_a.insert(pidx); }
          }
        } else if (layer == 18) {
          if (positive) {
            if (before && !pos_seen_r2_b.count(pidx)) { m_dc_pos.r2_before->Fill(edge); pos_seen_r2_b.insert(pidx); }
            if (!before && !pos_seen_r2_a.count(pidx) && (dc_pos_after.count(pidx))) { m_dc_pos.r2_after->Fill(edge); pos_seen_r2_a.insert(pidx); }
          } else {
            if (before && !neg_seen_r2_b.count(pidx)) { m_dc_neg.r2_before->Fill(edge); neg_seen_r2_b.insert(pidx); }
            if (!before && !neg_seen_r2_a.count(pidx) && (dc_neg_after.count(pidx))) { m_dc_neg.r2_after->Fill(edge); neg_seen_r2_a.insert(pidx); }
          }
        } else if (layer == 36) {
          if (positive) {
            if (before && !pos_seen_r3_b.count(pidx)) { m_dc_pos.r3_before->Fill(edge); pos_seen_r3_b.insert(pidx); }
            if (!before && !pos_seen_r3_a.count(pidx) && (dc_pos_after.count(pidx))) { m_dc_pos.r3_after->Fill(edge); pos_seen_r3_a.insert(pidx); }
          } else {
            if (before && !neg_seen_r3_b.count(pidx)) { m_dc_neg.r3_before->Fill(edge); neg_seen_r3_b.insert(pidx); }
            if (!before && !neg_seen_r3_a.count(pidx) && (dc_neg_after.count(pidx))) { m_dc_neg.r3_after->Fill(edge); neg_seen_r3_a.insert(pidx); }
          }
        }
      };

      // sign by pid sign from particle bank
      int pid = particle.getInt("pid", pidx);
      if (pid == 0 || pid == 22) continue;

      if (pid > 0) {
        if (dc_pos_before.count(pidx)) fill_dc(true,  true);
        if (dc_pos_after .count(pidx)) fill_dc(true,  false);
      } else {
        if (dc_neg_before.count(pidx)) fill_dc(false, true);
        if (dc_neg_after .count(pidx)) fill_dc(false, false);
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

void RGAFiducialFilterValidator::DrawCVTCanvas1x2(const char* title)
{
  if (!m_have_traj || !m_cvt_before || !m_cvt_after) return;

  auto* c = new TCanvas("rgafid_cvt_l12_all", title, 1200, 600);
  c->Divide(2,1);

  // Give the palette a bit more breathing room on the right
  const double left   = 0.12;
  const double right  = 0.16;
  const double bottom = 0.12;
  const double top    = 0.08;

  // compute survive %
  double pct = 0.0;
  if (m_cvt_before_n > 0) pct = 100.0 * double(m_cvt_after_n) / double(m_cvt_before_n);

  c->cd(1);
  gPad->SetLeftMargin(left);
  gPad->SetRightMargin(right);
  gPad->SetBottomMargin(bottom);
  gPad->SetTopMargin(top);
  // keep BEFORE title as-is
  m_cvt_before->Draw("COLZ");

  c->cd(2);
  gPad->SetLeftMargin(left);
  gPad->SetRightMargin(right);
  gPad->SetBottomMargin(bottom);
  gPad->SetTopMargin(top);
  // decorate AFTER with survive %
  m_cvt_after->SetTitle(Form("CVT layer 12 after (hadrons: #pm211,#pm321,#pm2212)  [survive = %.1f%%];phi (deg);theta (deg)", pct));
  m_cvt_after->Draw("COLZ");

  c->SaveAs(Form("%s_cvt_l12_phi_theta_hadrons.png", m_base.Data()));
}

void RGAFiducialFilterValidator::DrawDCCanvas2x3(const DCHists& H, bool positive, double survive_pct)
{
  auto* c = new TCanvas(positive ? "rgafid_dc_pos_2x3" : "rgafid_dc_neg_2x3",
                        positive ? "DC edges (+): before/after" : "DC edges (-): before/after",
                        1500, 900);
  c->Divide(3,2);

  auto setpad = [](){
    gPad->SetLeftMargin(0.12);
    gPad->SetRightMargin(0.06);
    gPad->SetBottomMargin(0.12);
    gPad->SetTopMargin(0.08);
  };

  // BEFORE row
  c->cd(1); setpad(); if (H.r1_before) { H.r1_before->SetLineWidth(2); H.r1_before->Draw("HIST"); H.r1_before->SetTitle("DC Region 1 (before);edge (cm);counts"); }
  c->cd(2); setpad(); if (H.r2_before) { H.r2_before->SetLineWidth(2); H.r2_before->Draw("HIST"); H.r2_before->SetTitle("DC Region 2 (before);edge (cm);counts"); }
  c->cd(3); setpad(); if (H.r3_before) { H.r3_before->SetLineWidth(2); H.r3_before->Draw("HIST"); H.r3_before->SetTitle("DC Region 3 (before);edge (cm);counts"); }

  // AFTER row with survive %
  c->cd(4); setpad(); if (H.r1_after)  { H.r1_after ->SetLineWidth(2); H.r1_after ->Draw("HIST"); H.r1_after ->SetTitle(Form("DC Region 1 (after)  [survive = %.1f%%];edge (cm);counts", survive_pct)); }
  c->cd(5); setpad(); if (H.r2_after)  { H.r2_after ->SetLineWidth(2); H.r2_after ->Draw("HIST"); H.r2_after ->SetTitle(Form("DC Region 2 (after)  [survive = %.1f%%];edge (cm);counts", survive_pct)); }
  c->cd(6); setpad(); if (H.r3_after)  { H.r3_after ->SetLineWidth(2); H.r3_after ->Draw("HIST"); H.r3_after ->SetTitle(Form("DC Region 3 (after)  [survive = %.1f%%];edge (cm);counts", survive_pct)); }

  c->SaveAs(Form("%s_dc_%s_2x3.png", m_base.Data(), positive ? "pos" : "neg"));
}

void RGAFiducialFilterValidator::Stop()
{
  // PCAL canvases
  DrawCalCanvas(11, "PCAL lv & lw (Electrons): kept solid, cut dashed");
  DrawCalCanvas(22, "PCAL lv & lw (Photons): kept solid, cut dashed");

  // FT 2x2
  DrawFTCanvas2x2();

  // CVT L12 1x2 (combined hadrons) with survive %
  DrawCVTCanvas1x2("CVT layer 12 (Hadrons): phi vs theta");

  // DC (pos & neg) 2x3 with survive %
  double pos_pct = (m_dc_pos_before_n>0) ? (100.0*double(m_dc_pos_after_n)/double(m_dc_pos_before_n)) : 0.0;
  double neg_pct = (m_dc_neg_before_n>0) ? (100.0*double(m_dc_neg_after_n)/double(m_dc_neg_before_n)) : 0.0;
  DrawDCCanvas2x3(m_dc_pos, true,  pos_pct);
  DrawDCCanvas2x3(m_dc_neg, false, neg_pct);

  if (m_out) {
    m_out->Write();
    m_log->Info("Wrote output file {}", m_out->GetName());
    m_out->Close();
  }
}

} // namespace iguana::clas12