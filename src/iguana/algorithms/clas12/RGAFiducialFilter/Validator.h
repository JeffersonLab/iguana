#pragma once

#include "iguana/algorithms/Validator.h"
#include "iguana/algorithms/TypeDefs.h"

#include <TH1.h>
#include <TH2.h>
#include <TFile.h>

#include <array>
#include <unordered_map>
#include <vector>

namespace iguana::clas12 {

// Validator:
//   - Runs RGAFiducialFilter
//   - PCAL (lv & lw) per sector (1..6), range [0,27]:
//       solid lines = kept (after cuts)
//       dashed lines = cut (removed by any cut)
//   - FT x-y: 2x2 grid (rows=e-/gamma, cols=before/after), with annulus & holes.
//   - CVT (layer 12): theta(deg) vs phi(deg) in a 1x2 (before | after).
class RGAFiducialFilterValidator : public Validator {
  DEFINE_IGUANA_VALIDATOR(RGAFiducialFilterValidator, clas12::RGAFiducialFilterValidator)

public:
  void Start(hipo::banklist& banks) override;
  void Run  (hipo::banklist& banks) const override;
  void Stop () override;

private:
  // banks
  hipo::banklist::size_type b_particle{};
  hipo::banklist::size_type b_calor{};
  hipo::banklist::size_type b_ft{};
  hipo::banklist::size_type b_traj{};
  bool m_have_calor = false;
  bool m_have_ft    = false;
  bool m_have_traj  = false;

  // algo sequence
  std::unique_ptr<AlgorithmSequence> m_seq;

  // PID rows we care about (for PCAL/FT plots)
  const std::array<int,2> kPIDs{11,22};

  // PCAL hists per PID & sector
  struct SecHists {
    TH1D* lv_kept=nullptr; TH1D* lv_cut=nullptr;
    TH1D* lw_kept=nullptr; TH1D* lw_cut=nullptr;
  };
  using PerPIDCal = std::array<SecHists, 7>; // index 1..6
  std::unordered_map<int, PerPIDCal> m_cal;

  // FT hists per PID (before/after)
  struct FTHists { TH2F* before=nullptr; TH2F* after=nullptr; };
  std::unordered_map<int, FTHists> m_ft_h;

  // CVT hists (before/after)
  TH2F* m_cvt_before = nullptr;
  TH2F* m_cvt_after  = nullptr;

  // FT overlay params (defaults match algorithm)
  struct FTDraw { float rmin=8.5f, rmax=15.5f; std::vector<std::array<float,3>> holes; };
  FTDraw m_ftdraw{};

  // output
  TString m_base;
  TFile*  m_out = nullptr;

  // helpers
  void BookIfNeeded();
  void LoadFTParamsFromYAML(); // guarded, optional
  void DrawCalCanvas(int pid, const char* title);
  void DrawFTCanvas2x2();
  void DrawCVTCanvas1x2();
};

} // namespace iguana::clas12