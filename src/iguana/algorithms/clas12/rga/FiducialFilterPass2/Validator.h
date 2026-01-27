#pragma once

#include "iguana/algorithms/TypeDefs.h" // (PDG names/titles if you want to expand later)
#include "iguana/algorithms/Validator.h"

#include <TFile.h>
#include <TH1.h>
#include <TH2.h>

#include <array>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>

namespace iguana::clas12::rga {

  /// @brief `iguana::clas12::rga::FiducialFilterPass2` validator
  class FiducialFilterPass2Validator : public Validator
  {
    DEFINE_IGUANA_VALIDATOR(FiducialFilterPass2Validator, clas12::rga::FiducialFilterPass2Validator)

    private: // hooks
      void StartHook(hipo::banklist& banks) override;
      bool RunHook(hipo::banklist& banks) const override;
      void StopHook() override;

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

      // ust run the algorithm via AlgorithmSequence
      std::unique_ptr<AlgorithmSequence> m_algo_seq;

      // Small PID set to visualize specially
      std::array<int, 2> const kPIDs{11, 22}; // electrons, photons

      // Histograms
      // PCAL hists per PID & sector (BEFORE/AFTER)
      struct SecHists {
          TH1D* lv_before = nullptr;
          TH1D* lv_after  = nullptr;
          TH1D* lw_before = nullptr;
          TH1D* lw_after  = nullptr;
      };
      using PerPIDCal = std::array<SecHists, 7>; // sectors 1..6
      std::unordered_map<int, PerPIDCal> m_cal;

      // Counts for survival % (unique pindex per sector)
      struct SecCounts {
          long long before = 0, after = 0;
      };
      using PerPIDCalCounts = std::array<SecCounts, 7>;
      std::unordered_map<int, PerPIDCalCounts> m_cal_counts;

      // FT XY before/after per PID
      struct FTHists {
          TH2F* before = nullptr;
          TH2F* after  = nullptr;
      };
      std::unordered_map<int, FTHists> m_ft_h;
      std::unordered_map<int, long long> m_ft_before_n, m_ft_after_n; // unique pindex counts

      // CVT (layer 12) phi vs theta (combined hadrons) before/after
      TH2F* m_cvt_before       = nullptr;
      TH2F* m_cvt_after        = nullptr;
      long long m_cvt_before_n = 0;
      long long m_cvt_after_n  = 0;

      // DC edge distributions by sign (pos/neg), regions 1,2,3; before/after
      struct DCHists {
          TH1D* r1_before = nullptr;
          TH1D* r2_before = nullptr;
          TH1D* r3_before = nullptr;
          TH1D* r1_after  = nullptr;
          TH1D* r2_after  = nullptr;
          TH1D* r3_after  = nullptr;
      };
      DCHists m_dc_pos{}, m_dc_neg{};
      long long m_dc_pos_before_n = 0, m_dc_pos_after_n = 0;
      long long m_dc_neg_before_n = 0, m_dc_neg_after_n = 0;

      // Torus polarity counters (for DC labelling only)
      long long m_torus_in_events  = 0;
      long long m_torus_out_events = 0;

      // Output
      TString m_base;
      TFile* m_out = nullptr;

      // Helpers
      void BookIfNeeded();
      void DrawCalCanvas(int pid, char const* title);
      void DrawFTCanvas2x2();
      void DrawCVTCanvas1x2(char const* title);
      void DrawDCCanvas2x3(DCHists const& H, char const* bend, double survive_pct);

      mutable std::mutex m_mutex{};
  };

}
