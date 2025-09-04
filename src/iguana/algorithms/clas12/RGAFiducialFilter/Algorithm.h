#pragma once

#include "iguana/algorithms/Algorithm.h"
#include "iguana/algorithms/TypeDefs.h"
#include "iguana/services/ConcurrentParam.h"

#include <optional>
#include <mutex>
#include <unordered_map>
#include <vector>
#include <array>
#include <utility>
#include <string>
#include <atomic>
#include <functional>

namespace iguana::clas12 {

  /// RGA fiducial filter with calorimeter + forward tagger cuts.
  ///
  /// Input banks: REC::Particle (tracks), REC::Calorimeter (optional),
  ///              REC::ForwardTagger (optional), RUN::config
  /// Output bank: REC::Particle (tracks)
  ///
  /// Strictness precedence (PCAL only):
  ///   SetStrictness() > env IGUANA_RGAFID_STRICTNESS > YAML [calorimeter.strictness[0]] > default(1)
  /// YAML is read only if IGUANA_RGAFID_USE_YAML=1.
  class RGAFiducialFilter : public Algorithm
  {
      DEFINE_IGUANA_ALGORITHM(RGAFiducialFilter, clas12::RGAFiducialFilter)

    public:
      struct CalHit { int sector=0; float lv=0, lw=0, lu=0; };
      struct CalLayers {
        std::vector<CalHit> L1;  // PCAL   (layer==1)
        std::vector<CalHit> L4;  // ECIN   (layer==4)
        std::vector<CalHit> L7;  // ECOUT  (layer==7)
        bool has_any = false;
      };

      void Start(hipo::banklist& banks) override;
      void Run  (hipo::banklist& banks) const override;
      void Stop () override;

      /// Prepare per-event cache (run-dependent)
      concurrent_key_t PrepareEvent(int runnum) const;

      /// Core filter: applies calorimeter cuts (with strictness) and forward-tagger cuts.
      /// Each detector's cuts are applied only if its bank is present.
      bool Filter(int track_index,
                  const hipo::bank* calBank,   // nullptr => skip cal cuts
                  const hipo::bank* ftBank,    // nullptr => skip FT cuts
                  concurrent_key_t key) const;

      /// Set strictness (1..3). Call BEFORE Start(). Values are clamped to [1,3].
      void SetStrictness(int strictness);

      // Accessors (thread-safe via ConcurrentParam)
      int GetRunNum(concurrent_key_t key) const;
      int GetCalStrictness(concurrent_key_t key) const;

    private:
      // Calorimeter helpers
      static CalLayers CollectCalHitsForTrack(const hipo::bank& calBank, int track_index);
      static bool PassCalStrictness(const CalLayers& h, int strictness);

      // cached masks: windows per sector/layer/axis
      using window_t = std::pair<float,float>;
      struct AxisWins { std::vector<window_t> lv, lw, lu; };
      struct SectorMasks { AxisWins pcal, ecin, ecout; };
      using MaskMap = std::unordered_map<int, SectorMasks>; // key = sector 1..6

      bool PassCalDeadPMTMasks(const CalLayers& h, concurrent_key_t key) const;
      MaskMap BuildCalMaskCache(int runnum) const;

      // Forward Tagger
      struct FTParams {
        float rmin = 8.5f;
        float rmax = 15.5f;
        std::vector<std::array<float,3>> holes; // {radius, cx, cy}
      };
      bool PassFTFiducial(int track_index, const hipo::bank* ftBank) const;

      /// Load per-run options (strictness from user/env/YAML; cal masks from YAML;
      /// FT parameters from YAML if present, else defaults).
      void Reload(int runnum, concurrent_key_t key) const;

      // bank indices (set only if present at Start)
      hipo::banklist::size_type b_particle{}, b_config{};
      hipo::banklist::size_type b_calor{}; bool m_have_calor = false;
      hipo::banklist::size_type b_ft{};    bool m_have_ft    = false;

      // cached (per-run) config
      mutable std::unique_ptr<ConcurrentParam<int>> o_runnum;
      mutable std::unique_ptr<ConcurrentParam<int>> o_cal_strictness;

      // mask cache per run
      mutable std::unordered_map<int, MaskMap> m_masks_by_run;

      // user-provided strictness (if any)
      std::optional<int> u_strictness_user;

      // FT parameters (global, not run-dependent)
      FTParams u_ft_params;

      // Debug controls (read once at Start)
      bool dbg_on = false;
      bool dbg_masks = false;
      bool dbg_ft = false;
      int  dbg_events = 0;         // how many track decisions to print
      mutable std::atomic<int> dbg_events_seen {0};

      // --- diag counters (printed in Stop) ---
      mutable std::atomic<long> c_pass{0}, c_fail_edge{0}, c_fail_mask{0}, c_fail_ft{0};

      // helpers
      static bool EnvOn(const char* name);
      static int  EnvInt(const char* name, int def);
      void DumpFTParams() const;
      void DumpMaskSummary(int runnum, const MaskMap& mm) const;

      mutable std::mutex m_mutex;
  };

} // namespace iguana::clas12