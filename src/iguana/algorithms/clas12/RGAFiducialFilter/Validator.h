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

/// Validator:
///   - Each subsystem's before/after is computed independently from the raw banks:
///       * PCAL (lv & lw) per sector (1-6), range [0,45], strictness s=1:
///           kept (solid) vs cut (dashed). 
///       * FT x-y: 2x2 grid (rows=e-/gamma, cols=before/after), with annulus & holes drawn.
///       * CVT layer 12 (detector==5): theta (y) vs phi (x),
///           single combined plot for hadron PIDs {±211, ±321, ±2212}, 1x2: before and after,
///           **After** includes survive %.
///       * DC (detector==6): two 2x3 canvases (Inb/Out). Columns=Region1/2/3 (layers 6/18/36),
///           rows=before and after.
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
  hipo::banklist::size_type b_config{}; 
  bool m_have_calor = false;
  bool m_have_ft    = false;
  bool m_have_traj  = false;

  // PID rows relevant for PCAL/FT displays
  const std::array<int,2> kPIDs{11,22};

  // PCAL hists per PID & sector
  struct SecHists {
    TH1D* lv_kept=nullptr; TH1D* lv_cut=nullptr;
    TH1D* lw_kept=nullptr; TH1D* lw_cut=nullptr;
  };
  using PerPIDCal = std::array<SecHists, 7>; // index 1-6
  std::unordered_map<int, PerPIDCal> m_cal;

  // PCAL per-PID, per-sector counts for survival %
  struct SecCounts { long long before=0, after=0; };
  using PerPIDCalCounts = std::array<SecCounts, 7>;
  std::unordered_map<int, PerPIDCalCounts> m_cal_counts;

  // FT hists per PID (before/after) + counters
  struct FTHists { TH2F* before=nullptr; TH2F* after=nullptr; };
  std::unordered_map<int, FTHists> m_ft_h;
  std::unordered_map<int, long long> m_ft_before_n, m_ft_after_n; // keyed by PID (11,22)

  // CVT combined hadron hists (before/after), layer 12: phi vs theta
  TH2F* m_cvt_before = nullptr;
  TH2F* m_cvt_after  = nullptr;
  long long m_cvt_before_n = 0;
  long long m_cvt_after_n  = 0;

  // DC edge distributions by sign (pos/neg), regions 1,2,3; before/after
  struct DCHists {
    TH1D* r1_before=nullptr; TH1D* r2_before=nullptr; TH1D* r3_before=nullptr;
    TH1D* r1_after =nullptr; TH1D* r2_after =nullptr; TH1D* r3_after =nullptr;
  };
  DCHists m_dc_pos{}, m_dc_neg{};
  long long m_dc_pos_before_n = 0, m_dc_pos_after_n = 0;
  long long m_dc_neg_before_n = 0, m_dc_neg_after_n = 0;

  // Torus polarity counters 
  long long m_torus_in_events  = 0;
  long long m_torus_out_events = 0;

  // FT overlay params 
  struct FTDraw { float rmin=0.f, rmax=0.f; std::vector<std::array<float,3>> holes; };
  FTDraw m_ftdraw{};

  // CVT/DC parameters loaded from YAML
  struct CVTParams {
    std::vector<int>    edge_layers;
    double              edge_min = 0.0;
    std::vector<double> phi_forbidden_deg;  // flattened pairs (open intervals)
  } m_cvt_params;

  struct DCParams {
    double theta_small_deg   = 0.0;
    double in_small_e1 = 0.0, in_small_e2 = 0.0, in_small_e3 = 0.0;
    double in_large_e1 = 0.0, in_large_e2 = 0.0, in_large_e3 = 0.0;
    double out_e1      = 0.0, out_e2      = 0.0, out_e3      = 0.0;
  } m_dc_params;

  // Cal strictness for validator’s before/after split
  int m_cal_strictness = 1;

  // output
  TString m_base;
  TFile*  m_out = nullptr;

  // helpers
  void BookIfNeeded();
  void LoadConfigFromYAML(); // REQUIRED: read all params for overlays and cuts
  void DrawCalCanvas(int pid, const char* title);
  void DrawFTCanvas2x2();
  void DrawCVTCanvas1x2(const char* title);
  void DrawDCCanvas2x3(const DCHists& H, const char* bend, double survive_pct);
};

}