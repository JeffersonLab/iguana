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
  ///   * Forward Tagger annulus + fixed circular hole vetoes.
  ///
  /// Defaults are read from Config.yaml. A caller may override strictness via
  /// SetStrictness(1|2|3) BEFORE Start().
  class RGAFiducialFilter : public Algorithm
  {
      DEFINE_IGUANA_ALGORITHM(RGAFiducialFilter, clas12::RGAFiducialFilter)

    public:
      void Start(hipo::banklist& banks) override;
      void Run  (hipo::banklist& banks) const override;
      void Stop () override;

      void SetStrictness(int strictness);

    private:
      // banks
      hipo::banklist::size_type b_particle{};
      hipo::banklist::size_type b_config{};
      hipo::banklist::size_type b_calor{};
      hipo::banklist::size_type b_ft{};
      bool m_have_calor = false;
      bool m_have_ft    = false;

      // config
      struct FTParams {
        float rmin = 8.5f;
        float rmax = 15.5f;
        std::vector<std::array<float,3>> holes; // {R,cx,cy}
      };
      FTParams m_ft{};
      int      m_strictness = 1;   // default (YAML can override)

      // helpers
      struct CalHit { int sector=0; float lv=0, lw=0, lu=0; int layer=0; };
      void LoadConfigFromYAML();
      static void CollectPCALHitsForTrack(const hipo::bank& cal, int pindex,
                                          std::vector<CalHit>& out_hits);
      bool PassPCalEdgeCuts(const std::vector<CalHit>& pcal_hits) const;
      bool PassFTFiducial(int track_index, const hipo::bank* ftBank) const;

      bool m_dbg = false;
  };

} // namespace iguana::clas12