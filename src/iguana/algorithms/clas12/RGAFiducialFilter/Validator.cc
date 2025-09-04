#include "Validator.h"
#include "iguana/services/YAMLReader.h"

#include <TCanvas.h>
#include <TLegend.h>
#include <TEllipse.h>

#include <set>
#include <unordered_map>

namespace iguana::clas12 {

REGISTER_IGUANA_VALIDATOR(RGAFiducialFilterValidator);

// small util
static bool banklist_has(hipo::banklist& banks, const char* name) {
  for (auto& b : banks) if (b.getSchema().getName() == name) return true;
  return false;
}

void RGAFiducialFilterValidator::LoadFTParamsFromYAML()
{
  // defaults already set
  ParseYAMLConfig();
  if (!GetConfig()) return;

  try {
    auto r = GetOptionVector<double>("clas12::RGAFiducialFilter.forward_tagger.radius");
    if (r.empty()) r = GetOptionVector<double>("forward_tagger.radius"); // allow bare
    if (r.size() >= 2) {
      float a = (float)r[0], b = (float)r[1];
      m_ftdraw.rmin = std::min(a,b); m_ftdraw.rmax = std::max(a,b);
    }
  } catch (...) {}

  try {
    auto flat = GetOptionVector<double>("clas12::RGAFiducialFilter.forward_tagger.holes_flat");
    if (flat.empty()) flat = GetOptionVector<double>("forward_tagger.holes_flat");
    m_ftdraw.holes.clear();
    for (size_t i=0;i+2<flat.size(); i+=3)
      m_ftdraw.holes.push_back({(float)flat[i], (float)flat[i+1], (float)flat[i+2]});
  } catch (...) {}
}

void RGAFiducialFilterValidator::BookIfNeeded()
{
  // PCAL: range 0..27 cm, 0.5 cm bins
  const int nb=54; const double lo=0, hi=27;

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
    F.before->SetStats(0); F.after->SetStats(0);
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
  if (banklist_has(banks, "REC::Calorimeter")) { b_calor = GetBankIndex(banks, "REC::Calorimeter"); m_have_calor=true; }
  if (banklist_has(banks, "REC::ForwardTagger")) { b_ft = GetBankIndex(banks, "REC::ForwardTagger"); m_have_ft=true; }

  // FT overlay params
  LoadFTParamsFromYAML();

  // Output
  if (auto dir = GetOutputDirectory()) {
    m_base = dir.value() + std::string("/rga_fiducial");
    m_out  = new TFile(m_base + ".root", "RECREATE");
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
      if (itb == before_pid.end()) continue; // not an e-/γ before
      if (cal.getInt("layer", i) != 1) continue; // PCAL only

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

    // styles
    H.lv_kept->SetLineColor(kBlue+1);  H.lv_kept->SetLineWidth(2);  H.lv_kept->SetLineStyle(1);
    H.lw_kept->SetLineColor(kRed+1);   H.lw_kept->SetLineWidth(2);  H.lw_kept->SetLineStyle(1);
    H.lv_cut ->SetLineColor(kBlue+1);  H.lv_cut ->SetLineWidth(2);  H.lv_cut ->SetLineStyle(2); // dashed
    H.lw_cut ->SetLineColor(kRed+1);   H.lw_cut ->SetLineWidth(2);  H.lw_cut ->SetLineStyle(2);

    // give an explicit title on lv kept
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

    // overlays: annulus + holes
    auto* outer = new TEllipse(0,0, m_ftdraw.rmax, m_ftdraw.rmax);
    auto* inner = new TEllipse(0,0, m_ftdraw.rmin, m_ftdraw.rmin);
    outer->SetFillStyle(0); inner->SetFillStyle(0);
    outer->SetLineStyle(2); inner->SetLineStyle(2);
    outer->Draw(); inner->Draw();
    for (auto const& H : m_ftdraw.holes) {
      auto* e = new TEllipse(H[1], H[2], H[0], H[0]);
      e->SetFillStyle(0); e->SetLineColor(kBlack); e->SetLineStyle(7); e->Draw();
    }

    auto* leg = new TLegend(0.58, 0.80, 0.90, 0.92);
    leg->SetBorderSize(0); leg->SetFillStyle(0);
    leg->AddEntry((TObject*)nullptr, "Annulus & holes shown", "");
    leg->Draw();
  };

  // electrons row
  draw_pad(1, m_ft_h.at(11).before, "Electrons (before cuts);x (cm);y (cm)");
  draw_pad(2, m_ft_h.at(11).after,  "Electrons (after cuts);x (cm);y (cm)");

  // photons row
  draw_pad(3, m_ft_h.at(22).before, "Photons (before cuts);x (cm);y (cm)");
  draw_pad(4, m_ft_h.at(22).after,  "Photons (after cuts);x (cm);y (cm)");

  c->SaveAs(Form("%s_ft_xy_2x2.png", m_base.Data()));
}

void RGAFiducialFilterValidator::Stop()
{
  // Draw PCAL canvases
  DrawCalCanvas(11, "PCAL lv & lw (Electrons): kept solid, cut dashed");
  DrawCalCanvas(22, "PCAL lv & lw (Photons): kept solid, cut dashed");

  // Draw FT 2x2
  DrawFTCanvas2x2();

  if (m_out) {
    m_out->Write();
    m_log->Info("Wrote output file {}", m_out->GetName());
    m_out->Close();
  }
}

} // namespace iguana::clas12