// Algorithm.h  (replace your current header with this)

#pragma once
#include "iguana/algorithms/Algorithm.h"
#include "iguana/algorithms/TypeDefs.h"
#include <array>
#include <vector>
#include <optional>

namespace iguana::clas12 {

class RGAFiducialFilter : public Algorithm
{
  DEFINE_IGUANA_ALGORITHM(RGAFiducialFilter, clas12::RGAFiducialFilter)

public:
  void Start(hipo::banklist& banks) override;
  void Run  (hipo::banklist& banks) const override;
  void Stop () override {}

  // Programmatic override (call before Start)
  void SetStrictness(int strictness);

private:
  // --- banks ---
  hipo::banklist::size_type b_particle{};
  hipo::banklist::size_type b_config{};
  hipo::banklist::size_type b_calor{};
  hipo::banklist::size_type b_ft{};
  bool m_have_calor = false;
  bool m_have_ft    = false;

  // --- config ---
  struct FTParams {
    float rmin = 8.5f;
    float rmax = 15.5f;
    std::vector<std::array<float,3>> holes; // {R,cx,cy}
  };
  FTParams m_ft{};
  int      m_strictness = 1;   // YAML default; SetStrictness can override
  bool     m_strictness_overridden = false;

  // --- debug env toggles ---
  static bool EnvOn (const char* name);
  static int  EnvInt(const char* name, int def);
  bool dbg_on=false, dbg_ft=false;
  int  dbg_events=0;

  // --- YAML ---
  void LoadConfigFromYAML();

  // --- helpers ---
  struct CalHit { int sector=0; float lv=0, lw=0, lu=0; int layer=0; };
  static void CollectPCALHitsForTrack(const hipo::bank& cal, int pindex,
                                      std::vector<CalHit>& out_hits);
  bool PassPCalEdgeCuts(const std::vector<CalHit>& pcal_hits) const;
  bool PassFTFiducial(int track_index, const hipo::bank* ftBank) const;
};

} // namespace iguana::clas12