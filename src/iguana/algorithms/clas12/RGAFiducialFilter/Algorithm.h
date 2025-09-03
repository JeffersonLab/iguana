#pragma once

#include "iguana/algorithms/Algorithm.h"
#include "iguana/algorithms/TypeDefs.h"
#include "iguana/services/ConcurrentParam.h"

#include <optional>
#include <mutex>
#include <unordered_map>
#include <vector>
#include <array>
#include <utility>   // std::pair
#include <memory>    // std::shared_ptr

namespace iguana::clas12 {

  class RGAFiducialFilter : public Algorithm
  {
      DEFINE_IGUANA_ALGORITHM(RGAFiducialFilter, clas12::RGAFiducialFilter)

    public:
      struct CalLayers {
        int   sector = 0;
        float lv1=0.f, lw1=0.f, lu1=0.f; // layer 1 (PCAL)
        float lv4=0.f, lw4=0.f, lu4=0.f; // layer 4 (ECin)
        float lv7=0.f, lw7=0.f, lu7=0.f; // layer 7 (ECout)
        bool  has_any = false;
      };

      void Start(hipo::banklist& banks) override;
      void Run  (hipo::banklist& banks) const override;
      void Stop () override;

      // Prepare per-event cache (run-dependent)
      concurrent_key_t PrepareEvent(int runnum) const;

      // Core filter
      bool Filter(int track_index,
                  const hipo::bank* calBank,
                  const hipo::bank* ftBank,
                  concurrent_key_t key) const;

      // Strictness setter (1..3)
      void SetStrictness(int strictness);

      // Accessors
      int GetRunNum(concurrent_key_t key) const;
      int GetCalStrictness(concurrent_key_t key) const;

    private:
      // Calorimeter helpers
      static CalLayers CollectCalHitsForTrack(const hipo::bank& calBank, int track_index);
      static bool PassCalStrictness(const CalLayers& h, int strictness);

      // Cached dead-PMT masks: windows per sector/layer/axis
      using window_t = std::pair<float,float>;
      struct AxisWins { std::vector<window_t> lv, lw, lu; };
      struct SectorMasks { AxisWins pcal, ecin, ecout; };
      using MaskMap = std::unordered_map<int, SectorMasks>; // key = sector 1..6

      bool     PassCalDeadPMTMasks(const CalLayers& h, concurrent_key_t key) const;
      MaskMap  BuildCalMaskCache(int runnum) const;

      // Forward Tagger helpers
      struct FTParams {
        float rmin = 8.5f;
        float rmax = 15.5f;
        std::vector<std::array<float,3>> holes; // {radius, cx, cy}
      };
      bool PassFTFiducial(int track_index, const hipo::bank* ftBank) const;

      // Per-run reload
      void Reload(int runnum, concurrent_key_t key) const;

      // Bank indices
      hipo::banklist::size_type b_particle{}, b_config{};
      hipo::banklist::size_type b_calor{}; bool m_have_calor = false;
      hipo::banklist::size_type b_ft{};    bool m_have_ft    = false;

      // Cached per-run params
      mutable std::unique_ptr<ConcurrentParam<int>>                          o_runnum;
      mutable std::unique_ptr<ConcurrentParam<int>>                          o_cal_strictness;
      mutable std::unique_ptr<ConcurrentParam<std::shared_ptr<MaskMap>>>     o_cal_masks;

      // User-provided strictness (if any)
      std::optional<int> u_strictness_user;

      // FT parameters (global, not run-dependent)
      FTParams u_ft_params;

      mutable std::mutex m_mutex;
  };

} // namespace iguana::clas12