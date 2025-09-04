#pragma once

#include "iguana/algorithms/Algorithm.h"
#include "iguana/algorithms/TypeDefs.h"
#include "iguana/services/ConcurrentParam.h"

#include <array>
#include <optional>
#include <string>
#include <vector>

namespace iguana::clas12 {

  /// RGA fiducial filter (minimal):
  ///   * PCAL-only edge cuts on lv & lw with strictness thresholds:
  ///       s=1 -> {lv,lw} >=  9.0 cm
  ///       s=2 -> {lv,lw} >= 13.5 cm
  ///       s=3 -> {lv,lw} >= 18.0 cm
  ///   * Forward Tagger annulus veto + fixed circular holes.
  ///
  /// Defaults (strictness and FT) are read from Config.yaml under:
  ///   clas12::RGAFiducialFilter:
  ///     calorimeter.strictness: [2]
  ///     forward_tagger.radius: [8.5, 15.5]
  ///     forward_tagger.holes_flat: [r1, cx1, cy1, r2, cx2, cy2, ...]
  ///
  /// A programmatic caller may override strictness by calling SetStrictness()
  /// BEFORE Start(). No environment variables are consulted for physics cuts.
  class RGAFiducialFilter : public Algorithm
  {
      DEFINE_IGUANA_ALGORITHM(RGAFiducialFilter, clas12::RGAFiducialFilter)

    public:
      void Start(hipo::banklist& banks) override;
      void Run  (hipo::banklist& banks) const override;
      void Stop () override;

      /// Optional runtime override: call BEFORE Start(). Value clamped to [1,3].
      void SetStrictness(int strictness);

    private:
      // --- data we read
      hipo::banklist::size_type b_particle{};
      hipo::banklist::size_type b_config{};
      hipo::banklist::size_type b_calor{};
      hipo::banklist::size_type b_ft{};
      bool m_have_calor = false;
      bool m_have_ft    = false;

      // --- config (from YAML; strictness optionally overridden via SetStrictness)
      struct FTParams {
        float rmin = 8.5f;
        float rmax = 15.5f;
        std::vector<std::array<float,3>> holes; // {radius, cx, cy}
      };
      FTParams m_ft{};
      int      m_strictness = 2;   // default; YAML may change; SetStrictness() may override

      // --- helpers
      struct CalHit { int sector=0; float lv=0, lw=0, lu=0; int layer=0; };
      void LoadConfigFromYAML(); // safe if YAML absent

      static void CollectPCALHitsForTrack(const hipo::bank& cal, int pindex,
                                          std::vector<CalHit>& out_hits);

      bool PassPCalEdgeCuts(const std::vector<CalHit>& pcal_hits) const;
      bool PassFTFiducial(int track_index, const hipo::bank* ftBank) const;

      // small debug toggle (optional)
      bool m_dbg = false;
  };

} // namespace iguana::clas12