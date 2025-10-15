#include "Validator.h"

#include <TCanvas.h>
#include <TLegend.h>
#include <TString.h>
#include <TPad.h>
#include <TROOT.h>
#include <TStyle.h>
#include <TH1.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace iguana::clas12 {

REGISTER_IGUANA_VALIDATOR(RGAFiducialFilterValidator);

static bool banklist_has(hipo::banklist& banks, const char* name) {
  for (auto& b : banks) if (b.getSchema().getName() == name) return true;
  return false;
}

static inline void SaveAndDisposeCanvas(TCanvas* c, const char* path_png) {
  if (!c) return;
  c->Modified();
  c->Update();
  c->Print(path_png);
  // Remove from global list before deletion
  if (gROOT && gROOT->GetListOfCanvases())
    gROOT->GetListOfCanvases()->Remove(c);
  delete c;
}

void RGAFiducialFilterValidator::BookIfNeeded() {
  // PCal: 0–45 cm with 4.5 cm bins (bar width)
  const int nb = 10;
  const double lo = 0.0, hi = 45.0;

  for (int pid : kPIDs) {
    auto& P = m_cal[pid];
    for (int s=1; s<=6; ++s) {
      if (!P[s].lv_before) {
        P[s].lv_before = new TH1D(Form("h_pcal_lv_before_pid%d_s%d", pid, s),
          Form("PID %d S%d;length (cm);counts", pid, s), nb, lo, hi);
        P[s].lv_before->SetStats(0);
        P[s].lv_before->SetDirectory(nullptr);
      }
      if (!P[s].lv_after) {
        P[s].lv_after = new TH1D(Form("h_pcal_lv_after_pid%d_s%d", pid, s),
          Form("PID %d S%d;length (cm);counts", pid, s), nb, lo, hi);
        P[s].lv_after->SetStats(0);
        P[s].lv_after->SetDirectory(nullptr);
      }
      if (!P[s].lw_before) {
        P[s].lw_before = new TH1D(Form("h_pcal_lw_before_pid%d_s%d", pid, s),
          Form("PID %d S%d;length (cm);counts", pid, s), nb, lo, hi);
        P[s].lw_before->SetStats(0);
        P[s].lw_before->SetDirectory(nullptr);
      }
      if (!P[s].lw_after) {
        P[s].lw_after = new TH1D(Form("h_pcal_lw_after_pid%d_s%d", pid, s),
          Form("PID %d S%d;length (cm);counts", pid, s), nb, lo, hi);
        P[s].lw_after->SetStats(0);
        P[s].lw_after->SetDirectory(nullptr);
      }
    }
  }

  // FT: x,y in +/-20 cm
  for (int pid : kPIDs) {
    auto& F = m_ft_h[pid];
    if (!F.before) {
      F.before = new TH2F(Form("h_ft_before_pid%d", pid),
        Form("FT x-y before (PID %d);x (cm);y (cm)", pid), 120, -20, 20, 120, -20, 20);
      F.before->SetStats(0);
      F.before->SetDirectory(nullptr);
    }
    if (!F.after) {
      F.after  = new TH2F(Form("h_ft_after_pid%d", pid),
        Form("FT x-y after (PID %d);x (cm);y (cm)", pid), 120, -20, 20, 120, -20, 20);
      F.after->SetStats(0);
      F.after->SetDirectory(nullptr);
    }
  }

  // CVT layer 12: phi (deg) vs theta (deg), hadrons only
  if (!m_cvt_before) {
    m_cvt_before = new TH2F("h_cvt_l12_phi_theta_before_all",
      "CVT layer 12 before (hadrons: #pm211,#pm321,#pm2212);phi (deg);theta (deg)",
      180, 0, 360,  90, 0, 90);
    m_cvt_before->SetStats(0);
    m_cvt_before->SetDirectory(nullptr);
  }
  if (!m_cvt_after) {
    m_cvt_after  = new TH2F("h_cvt_l12_phi_theta_after_all",
      "CVT layer 12 after (hadrons: #pm211,#pm321,#pm2212);phi (deg);theta (deg)",
      180, 0, 360,  90, 0, 90);
    m_cvt_after->SetStats(0);
    m_cvt_after->SetDirectory(nullptr);
  }

  // DC edges: 0–30 cm in 0.3 cm bins
  auto mk = [](const char* name, const char* title){
    auto* h = new TH1D(name, title, 100, 0.0, 30.0);
    h->SetStats(0);
    h->SetDirectory(nullptr);
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

void RGAFiducialFilterValidator::Start(hipo::banklist& banks) {

  if (gROOT) gROOT->SetBatch(kTRUE);
  if (gStyle) gStyle->SetOptStat(0);
  TH1::AddDirectory(kFALSE);

  b_particle = GetBankIndex(banks, "REC::Particle");
  if (banklist_has(banks, "REC::Calorimeter")) {
    b_calor = GetBankIndex(banks, "REC::Calorimeter"); m_have_calor=true;
  }
  if (banklist_has(banks, "REC::ForwardTagger")) {
    b_ft = GetBankIndex(banks, "REC::ForwardTagger"); m_have_ft=true;
  }
  if (banklist_has(banks, "REC::Traj")) {
    b_traj = GetBankIndex(banks, "REC::Traj"); m_have_traj=true;
  } else {
    m_log->Info("[RGAFID][VAL] REC::Traj not provided; CVT/DC plots disabled. "
      "Re-run with -b REC::Traj to enable trajectory-based plots.");
  }
  b_config = GetBankIndex(banks, "RUN::config");

  m_algo_seq = std::make_unique<AlgorithmSequence>();
  m_algo_seq->Add("clas12::RGAFiducialFilter");
  m_algo_seq->Start(banks);

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

// DC pad margins
static inline void SetDCPadMargins() {
  gPad->SetLeftMargin(0.16);
  gPad->SetRightMargin(0.06);
  gPad->SetBottomMargin(0.12);
  gPad->SetTopMargin(0.08);
}

bool RGAFiducialFilterValidator::Run(hipo::banklist& banks) const {
  auto& particle = GetBank(banks, b_particle, "REC::Particle");
  auto& config   = GetBank(banks, b_config,   "RUN::config");

  // track torus polarity stats (labels for DC summary)
  {
    bool e_out = (config.getFloat("torus", 0) == 1.0f);
    if (e_out) const_cast<RGAFiducialFilterValidator*>(this)->m_torus_out_events++;
    else       const_cast<RGAFiducialFilterValidator*>(this)->m_torus_in_events++;
  }

  // snapshot before sets of pindex
  std::unordered_set<int> eorg_before;  
  std::unordered_set<int> had_before;
  std::unordered_set<int> pos_before, neg_before;

  for (auto const& row : particle.getRowList()) {
    int pidx = (int)row;
    int pid  = particle.getInt("pid", row);
    if (pid==11 || pid==22) eorg_before.insert(pidx);
    if (pid==211 || pid==321 || pid==2212 || pid==-211 || pid==-321 || pid==-2212)
      had_before.insert(pidx);
    if (pid>0) pos_before.insert(pidx);
    else if (pid<0) neg_before.insert(pidx);
  }

  std::unordered_map<int,int> pid_before;
  for (auto const& row : particle.getRowList()) {
    pid_before[(int)row] = particle.getInt("pid", row);
  }

  // run the algorithm to prune REC::Particle in place
  m_algo_seq->Run(banks);

  // Snapshot after sets
  std::unordered_set<int> eorg_after;
  std::unordered_set<int> had_after;
  std::unordered_set<int> pos_after, neg_after;

  for (auto const& row : particle.getRowList()) {
    int pidx = (int)row;
    int pid  = particle.getInt("pid", row);
    if (pid==11 || pid==22) eorg_after.insert(pidx);
    if (pid==211 || pid==321 || pid==2212 || pid==-211 || pid==-321 || pid==-2212)
      had_after.insert(pidx);
    if (pid>0) pos_after.insert(pidx);
    else if (pid<0) neg_after.insert(pidx);
  }

  std::scoped_lock<std::mutex> lock(m_mutex);

  // PCal before/after (electrons, photons)
  if (m_have_calor) {
    auto& cal = GetBank(banks, b_calor, "REC::Calorimeter");

    // unique pindex counters per sector (for survival %)
    std::array<std::set<int>,7> be_e, af_e, be_g, af_g;

    const int n = cal.getRows();
    for (int i=0;i<n;++i) {
      if (cal.getInt("layer", i) != 1) continue; // PCal only
      int pidx = cal.getInt("pindex", i);
      if (!eorg_before.count(pidx)) continue;

      int sector = cal.getInt("sector", i);
      if (sector < 1 || sector > 6) continue;

      double lv = cal.getFloat("lv", i);
      double lw = cal.getFloat("lw", i);

      // PID for this pindex (from before snapshot)
      int pid = 0;
      auto itpb = pid_before.find(pidx);
      if (itpb == pid_before.end()) continue;
      pid = itpb->second;
      if (pid!=11 && pid!=22) continue;

      auto& H = const_cast<RGAFiducialFilterValidator*>(this)->m_cal[pid][sector];

      if (lv >= 0.0 && lv <= 45.0) H.lv_before->Fill(lv);
      if (lw >= 0.0 && lw <= 45.0) H.lw_before->Fill(lw);

      const bool survived = eorg_after.count(pidx);

      if (survived) {
        if (lv >= 0.0 && lv <= 45.0) H.lv_after->Fill(lv);
        if (lw >= 0.0 && lw <= 45.0) H.lw_after->Fill(lw);
      }

      // unique pindex per sector (counts)
      if (pid==11) {
        be_e[sector].insert(pidx);
        if (survived) af_e[sector].insert(pidx);
      } else {
        be_g[sector].insert(pidx);
        if (survived) af_g[sector].insert(pidx);
      }
    }

    for (int s=1;s<=6;++s) {
      const_cast<RGAFiducialFilterValidator*>(this)->m_cal_counts[11][s].before += be_e[s].size();
      const_cast<RGAFiducialFilterValidator*>(this)->m_cal_counts[11][s].after  += af_e[s].size();
      const_cast<RGAFiducialFilterValidator*>(this)->m_cal_counts[22][s].before += be_g[s].size();
      const_cast<RGAFiducialFilterValidator*>(this)->m_cal_counts[22][s].after  += af_g[s].size();
    }
  }

  // FT XY before/after (e-/gamma)
  if (m_have_ft) {
    auto& ft = GetBank(banks, b_ft, "REC::ForwardTagger");

    std::set<int> seen_b_e, seen_a_e, seen_b_g, seen_a_g;

    const int n = ft.getRows();
    for (int i=0;i<n;++i) {
      int pidx = ft.getInt("pindex", i);
      if (!eorg_before.count(pidx)) continue;

      int pid = 0;
      auto itpb = pid_before.find(pidx);
      if (itpb == pid_before.end()) continue;
      pid = itpb->second;
      if (pid!=11 && pid!=22) continue;

      auto& HH = const_cast<RGAFiducialFilterValidator*>(this)->m_ft_h.at(pid);

      double x = ft.getFloat("x", i);
      double y = ft.getFloat("y", i);
      if (pid==11) {
        if (!seen_b_e.count(pidx)) { HH.before->Fill(x, y); seen_b_e.insert(pidx); }
        if (eorg_after.count(pidx) && !seen_a_e.count(pidx)) { HH.after->Fill(x, y); seen_a_e.insert(pidx); }
      } else {
        if (!seen_b_g.count(pidx)) { HH.before->Fill(x, y); seen_b_g.insert(pidx); }
        if (eorg_after.count(pidx) && !seen_a_g.count(pidx)) { HH.after->Fill(x, y); seen_a_g.insert(pidx); }
      }
    }

    const_cast<RGAFiducialFilterValidator*>(this)->m_ft_before_n[11] += seen_b_e.size();
    const_cast<RGAFiducialFilterValidator*>(this)->m_ft_after_n [11] += seen_a_e.size();
    const_cast<RGAFiducialFilterValidator*>(this)->m_ft_before_n[22] += seen_b_g.size();
    const_cast<RGAFiducialFilterValidator*>(this)->m_ft_after_n [22] += seen_a_g.size();
  }

  // CVT layer 12 phi/theta before/after (hadrons)
  if (m_have_traj) {
    auto& traj = GetBank(banks, b_traj, "REC::Traj");

    std::set<int> b_seen, a_seen;

    const int n = traj.getRows();
    for (int i=0; i<n; ++i) {
      if (traj.getInt("detector", i) != 5) continue; // CVT
      if (traj.getInt("layer", i)    != 12) continue;

      int pidx = traj.getInt("pindex", i);
      if (!had_before.count(pidx)) continue;

      double x = traj.getFloat("x", i);
      double y = traj.getFloat("y", i);
      double z = traj.getFloat("z", i);

      constexpr double kPI = 3.14159;
      double phi = std::atan2(y, x) * (180.0/kPI); if (phi < 0) phi += 360.0;
      double rho = std::sqrt(x*x + y*y);
      double theta = std::atan2(rho, (z==0.0 ? 1e-9 : z)) * (180.0/kPI);

      if (!b_seen.count(pidx)) { m_cvt_before->Fill(phi, theta); b_seen.insert(pidx); }
      if (had_after.count(pidx) && !a_seen.count(pidx)) {
        m_cvt_after->Fill(phi, theta); a_seen.insert(pidx);
      }
    }

    const_cast<RGAFiducialFilterValidator*>(this)->m_cvt_before_n += (long long) b_seen.size();
    const_cast<RGAFiducialFilterValidator*>(this)->m_cvt_after_n  += (long long) a_seen.size();
  }

  // DC edges pos/neg before/after (use last-seen edge per (track,region))
  if (m_have_traj) {
    auto& traj = GetBank(banks, b_traj, "REC::Traj");

    std::unordered_map<int,double> pos_r1, pos_r2, pos_r3;
    std::unordered_map<int,double> neg_r1, neg_r2, neg_r3;

    const int n = traj.getRows();
    for (int i=0;i<n;++i) {
      if (traj.getInt("detector", i) != 6) continue; // DC

      int pidx   = traj.getInt("pindex", i);
      double edge = traj.getFloat("edge", i);
      int layer   = traj.getInt("layer", i);

      bool is_pos_before = pos_before.count(pidx);
      bool is_neg_before = neg_before.count(pidx);
      if (!is_pos_before && !is_neg_before) continue;

      if (is_pos_before) {
        if      (layer== 6) pos_r1[pidx] = edge;
        else if (layer==18) pos_r2[pidx] = edge;
        else if (layer==36) pos_r3[pidx] = edge;
      } else { // negative
        if      (layer== 6) neg_r1[pidx] = edge;
        else if (layer==18) neg_r2[pidx] = edge;
        else if (layer==36) neg_r3[pidx] = edge;
      }
    }

    for (auto& kv : pos_r1) { if (m_dc_pos.r1_before) m_dc_pos.r1_before->Fill(kv.second); if (pos_after.count(kv.first) && m_dc_pos.r1_after) m_dc_pos.r1_after->Fill(kv.second); }
    for (auto& kv : pos_r2) { if (m_dc_pos.r2_before) m_dc_pos.r2_before->Fill(kv.second); if (pos_after.count(kv.first) && m_dc_pos.r2_after) m_dc_pos.r2_after->Fill(kv.second); }
    for (auto& kv : pos_r3) { if (m_dc_pos.r3_before) m_dc_pos.r3_before->Fill(kv.second); if (pos_after.count(kv.first) && m_dc_pos.r3_after) m_dc_pos.r3_after->Fill(kv.second); }
    for (auto& kv : neg_r1) { if (m_dc_neg.r1_before) m_dc_neg.r1_before->Fill(kv.second); if (neg_after.count(kv.first) && m_dc_neg.r1_after) m_dc_neg.r1_after->Fill(kv.second); }
    for (auto& kv : neg_r2) { if (m_dc_neg.r2_before) m_dc_neg.r2_before->Fill(kv.second); if (neg_after.count(kv.first) && m_dc_neg.r2_after) m_dc_neg.r2_after->Fill(kv.second); }
    for (auto& kv : neg_r3) { if (m_dc_neg.r3_before) m_dc_neg.r3_before->Fill(kv.second); if (neg_after.count(kv.first) && m_dc_neg.r3_after) m_dc_neg.r3_after->Fill(kv.second); }

    auto set_from_keys = [](const std::unordered_map<int,double>& m){
      std::set<int> s; for (auto& kv : m) s.insert(kv.first); return s; };

    auto inter3 = [](const std::set<int>& A, 
        const std::set<int>& B, const std::set<int>& C)->size_t {
      const std::set<int>* smallest = &A;
      if (B.size() < smallest->size()) smallest = &B;
      if (C.size() < smallest->size()) smallest = &C;
      size_t cnt=0;
      for (int v : *smallest) if (A.count(v) && B.count(v) && C.count(v)) ++cnt;
      return cnt;
    };

    auto keep_if_survived = [](const std::set<int>& S, const std::unordered_set<int>& survivors){
      std::set<int> out; for (int v : S) if (survivors.count(v)) out.insert(v); return out; };

    std::set<int> pos_b1 = set_from_keys(pos_r1), pos_b2 = set_from_keys(pos_r2), pos_b3 = set_from_keys(pos_r3);
    std::set<int> neg_b1 = set_from_keys(neg_r1), neg_b2 = set_from_keys(neg_r2), neg_b3 = set_from_keys(neg_r3);

    std::set<int> pos_a1 = keep_if_survived(pos_b1, pos_after);
    std::set<int> pos_a2 = keep_if_survived(pos_b2, pos_after);
    std::set<int> pos_a3 = keep_if_survived(pos_b3, pos_after);
    std::set<int> neg_a1 = keep_if_survived(neg_b1, neg_after);
    std::set<int> neg_a2 = keep_if_survived(neg_b2, neg_after);
    std::set<int> neg_a3 = keep_if_survived(neg_b3, neg_after);

    const_cast<RGAFiducialFilterValidator*>(this)->m_dc_pos_before_n += (long long) inter3(pos_b1, pos_b2, pos_b3);
    const_cast<RGAFiducialFilterValidator*>(this)->m_dc_pos_after_n  += (long long) inter3(pos_a1, pos_a2, pos_a3);
    const_cast<RGAFiducialFilterValidator*>(this)->m_dc_neg_before_n += (long long) inter3(neg_b1, neg_b2, neg_b3);
    const_cast<RGAFiducialFilterValidator*>(this)->m_dc_neg_after_n  += (long long) inter3(neg_a1, neg_a2, neg_a3);
  }
  return true;
}

// plotting
void RGAFiducialFilterValidator::DrawCalCanvas(int pid, const char* title) {
  auto it = m_cal.find(pid);
  if (it == m_cal.end()) return;

  auto* c = new TCanvas(Form("rgafid_pcal_pid%d", pid), title, 1400, 900);
  c->Divide(3,2);

  for (int s=1; s<=6; ++s) {
    c->cd(s);
    gPad->SetLeftMargin(0.12); gPad->SetRightMargin(0.04);
    gPad->SetBottomMargin(0.12); gPad->SetTopMargin(0.08);

    auto& H = it->second[s];
    if (!H.lv_before) continue;

    H.lv_before->SetLineColor(kBlue+1);  H.lv_before->SetLineWidth(2);
    H.lw_before->SetLineColor(kRed+1);   H.lw_before->SetLineWidth(2);
    H.lv_after ->SetLineColor(kBlue+1);  H.lv_after ->SetLineWidth(2);  H.lv_after ->SetLineStyle(2);
    H.lw_after ->SetLineColor(kRed+1);   H.lw_after ->SetLineWidth(2);  H.lw_after ->SetLineStyle(2);

    // survival % for this PID/sector (unique pindex)
    long long b = m_cal_counts[pid][s].before;
    long long a = m_cal_counts[pid][s].after;
    double pct = (b>0) ? (100.0*double(a)/double(b)) : 0.0;

    H.lv_before->SetTitle(Form("%s - Sector %d  [survive = %.3f%%];length (cm);counts",
      pid==11?"Electrons":"Photons", s, pct));

    H.lv_before->Draw("HIST");
    H.lw_before->Draw("HISTSAME");
    H.lv_after ->Draw("HISTSAME");
    H.lw_after ->Draw("HISTSAME");

    auto* leg = new TLegend(0.55, 0.72, 0.88, 0.90);
    leg->SetBorderSize(0); leg->SetFillStyle(0);
    leg->AddEntry(H.lv_before, "lv before", "l");
    leg->AddEntry(H.lw_before, "lw before", "l");
    leg->AddEntry(H.lv_after , "lv after",  "l");
    leg->AddEntry(H.lw_after , "lw after",  "l");
    leg->SetBit(TObject::kCanDelete); 
    leg->Draw();
  }

  SaveAndDisposeCanvas(c, Form("%s_pcal_lv_lw_pid%d.png", m_base.Data(), pid));
}

void RGAFiducialFilterValidator::DrawFTCanvas2x2() {
  if (!m_have_ft) return;

  auto* c = new TCanvas("rgafid_ft_xy_2x2", "FT x-y Before/After", 1200, 900);
  c->Divide(2,2);

  auto draw_pad = [&](int pad, TH2F* h, const char* ttl){
    c->cd(pad);
    gPad->SetLeftMargin(0.12); gPad->SetRightMargin(0.04);
    gPad->SetBottomMargin(0.12); gPad->SetTopMargin(0.08);
    h->SetTitle(ttl);
    h->Draw("COLZ");
  };

  auto pct = [&](int pid)->double{
    long long b = m_ft_before_n[pid];
    long long a = m_ft_after_n[pid];
    return (b>0) ? (100.0*double(a)/double(b)) : 0.0;
  };

  draw_pad(1, m_ft_h.at(11).before, "Electrons (before);x (cm);y (cm)");
  draw_pad(2, m_ft_h.at(11).after,
    Form("Electrons (after)  [survive = %.3f%%];x (cm);y (cm)", pct(11)));

  draw_pad(3, m_ft_h.at(22).before, "Photons (before);x (cm);y (cm)");
  draw_pad(4, m_ft_h.at(22).after,
    Form("Photons (after)  [survive = %.3f%%];x (cm);y (cm)", pct(22)));

  SaveAndDisposeCanvas(c, Form("%s_ft_xy_2x2.png", m_base.Data()));
}

void RGAFiducialFilterValidator::DrawCVTCanvas1x2(const char* title) {
  if (!m_have_traj || !m_cvt_before || !m_cvt_after) return;

  auto* c = new TCanvas("rgafid_cvt_l12_all", title, 1200, 600);
  c->Divide(2,1);

  const double left   = 0.12;
  const double right  = 0.16;
  const double bottom = 0.12;
  const double top    = 0.08;

  double pct = 0.0;
  if (m_cvt_before_n > 0) pct = 100.0 * double(m_cvt_after_n) / double(m_cvt_before_n);

  c->cd(1);
  gPad->SetLeftMargin(left);
  gPad->SetRightMargin(right);
  gPad->SetBottomMargin(bottom);
  gPad->SetTopMargin(top);
  m_cvt_before->Draw("COLZ");

  c->cd(2);
  gPad->SetLeftMargin(left);
  gPad->SetRightMargin(right);
  gPad->SetBottomMargin(bottom);
  gPad->SetTopMargin(top);
  m_cvt_after->SetTitle(Form("CVT layer 12 after (hadrons)  [survive = %.3f%%];phi (deg);theta (deg)", pct));
  m_cvt_after->Draw("COLZ");

  SaveAndDisposeCanvas(c, Form("%s_cvt_l12_phi_theta_hadrons.png", m_base.Data()));
}

void RGAFiducialFilterValidator::DrawDCCanvas2x3(const DCHists& H,
  const char* bend, double survive_pct) {

  TString bendTitle = (TString(bend)=="inb") ? "Inb" : "Out";

  auto* c = new TCanvas(Form("rgafid_dc_%s_2x3", bend),
    Form("%s DC edges: before/after", bendTitle.Data()), 1500, 900);
  c->Divide(3,2);

  // before row
  c->cd(1); SetDCPadMargins(); if (H.r1_before) {
    H.r1_before->SetLineWidth(2);
    H.r1_before->Draw("HIST");
    H.r1_before->SetTitle(Form("%s DC Region 1 (before);edge (cm);counts", bendTitle.Data())); }
  c->cd(2); SetDCPadMargins(); if (H.r2_before) {
    H.r2_before->SetLineWidth(2);
    H.r2_before->Draw("HIST");
    H.r2_before->SetTitle(Form("%s DC Region 2 (before);edge (cm);counts", bendTitle.Data())); }
  c->cd(3); SetDCPadMargins(); if (H.r3_before) {
    H.r3_before->SetLineWidth(2);
    H.r3_before->Draw("HIST");
    H.r3_before->SetTitle(Form("%s DC Region 3 (before);edge (cm);counts", bendTitle.Data())); }

  // after row 
  c->cd(4); SetDCPadMargins(); if (H.r1_after)  {
    H.r1_after ->SetLineWidth(2);
    H.r1_after ->Draw("HIST");
    H.r1_after ->SetTitle(Form("%s DC Region 1 (after)  [survive = %.3f%%];edge (cm);counts", bendTitle.Data(), survive_pct)); }
  c->cd(5); SetDCPadMargins(); if (H.r2_after)  {
    H.r2_after ->SetLineWidth(2);
    H.r2_after ->Draw("HIST");
    H.r2_after ->SetTitle(Form("%s DC Region 2 (after)  [survive = %.3f%%];edge (cm);counts", bendTitle.Data(), survive_pct)); }
  c->cd(6); SetDCPadMargins(); if (H.r3_after)  {
    H.r3_after ->SetLineWidth(2);
    H.r3_after ->Draw("HIST");
    H.r3_after ->SetTitle(Form("%s DC Region 3 (after)  [survive = %.3f%%];edge (cm);counts", bendTitle.Data(), survive_pct)); }

  SaveAndDisposeCanvas(c, Form("%s_dc_%s_2x3.png", m_base.Data(), bend));
}

void RGAFiducialFilterValidator::Stop() {
  // PCAL canvases
  DrawCalCanvas(11, "PCAL lv & lw (Electrons): before solid, after dashed");
  DrawCalCanvas(22, "PCAL lv & lw (Photons): before solid, after dashed");

  // FT 2x2
  DrawFTCanvas2x2();

  // CVT 1x2 (combined hadrons) with survive %
  DrawCVTCanvas1x2("CVT layer 12 (Hadrons): phi vs theta");

  // Torus labels for DC summaries
  bool electron_out = (m_torus_out_events >= m_torus_in_events);
  const char* pos_bend_id = electron_out ? "inb" : "out";
  const char* neg_bend_id = electron_out ? "out" : "inb";

  // DC canvases
  double pos_pct =
    (m_dc_pos_before_n>0) ? (100.0*double(m_dc_pos_after_n)/double(m_dc_pos_before_n)) : 0.0;
  double neg_pct =
    (m_dc_neg_before_n>0) ? (100.0*double(m_dc_neg_after_n)/double(m_dc_neg_before_n)) : 0.0;
  DrawDCCanvas2x3(m_dc_pos, pos_bend_id, pos_pct);
  DrawDCCanvas2x3(m_dc_neg, neg_bend_id, neg_pct);

  // write all histograms into the ROOT file 
  if (m_out) {
    m_out->cd();

    auto write_obj = [&](TObject* o){ if (o) o->Write(); };

    // PCAL (per PID, per sector)
    for (auto& kv : m_cal) {
      auto& sectors = kv.second;
      for (int s=1; s<=6; ++s) {
        auto& H = sectors[s];
        write_obj(H.lv_before);
        write_obj(H.lv_after);
        write_obj(H.lw_before);
        write_obj(H.lw_after);
      }
    }

    // FT
    for (auto& kv : m_ft_h) {
      write_obj(kv.second.before);
      write_obj(kv.second.after);
    }

    // CVT
    write_obj(m_cvt_before);
    write_obj(m_cvt_after);

    // DC
    auto write_dc = [&](const DCHists& H){
      write_obj(H.r1_before); write_obj(H.r1_after);
      write_obj(H.r2_before); write_obj(H.r2_after);
      write_obj(H.r3_before); write_obj(H.r3_after);
    };
    write_dc(m_dc_pos);
    write_dc(m_dc_neg);

    m_out->Write();
    m_log->Info("Wrote output file {}", m_out->GetName());
    m_out->Close();
    delete m_out;
    m_out = nullptr;
  }

  // explicitly delete all booked histograms 
  auto zap = [&](auto*& p){ if (p) { delete p; p = nullptr; } };

  // PCAL
  for (auto& kv : m_cal) {
    auto& sectors = kv.second;
    for (int s=1; s<=6; ++s) {
      auto& H = sectors[s];
      zap(H.lv_before); zap(H.lv_after);
      zap(H.lw_before); zap(H.lw_after);
    }
  }
  m_cal.clear();

  // FT
  for (auto& kv : m_ft_h) {
    zap(kv.second.before);
    zap(kv.second.after);
  }
  m_ft_h.clear();

  // CVT
  zap(m_cvt_before);
  zap(m_cvt_after);

  // DC
  auto zap_dc = [&](DCHists& H){
    zap(H.r1_before); zap(H.r1_after);
    zap(H.r2_before); zap(H.r2_after);
    zap(H.r3_before); zap(H.r3_after);
  };
  zap_dc(m_dc_pos);
  zap_dc(m_dc_neg);

  // ensure no canvases linger globally
  if (gROOT && gROOT->GetListOfCanvases())
    gROOT->GetListOfCanvases()->Delete();
}

}
