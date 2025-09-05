#pragma once

#include "iguana/algorithms/Algorithm.h"
#include "iguana/algorithms/TypeDefs.h"
#include "iguana/services/ConcurrentParam.h"

#include <array>
#include <atomic>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

namespace iguana::clas12 {

// Minimal RGA fiducial filter:
//   - PCAL-only edge cuts on lv & lw with strictness thresholds:
//       s=1 -> {lv,lw} >=  9.0 cm
//       s=2 -> {lv,lw} >= 13.5 cm
//       s=3 -> {lv,lw} >= 18.0 cm
//   - Forward Tagger annulus + circular hole vetoes.
//   - CVT: edge > edge_min on selected layers, and forbidden phi wedges (always applied).
// Defaults are read from Config.yaml (same dir). A caller may override
// strictness via SetStrictness(1|2|3) BEFORE Start().
class RGAFiducialFilter : public Algorithm {
  DEFINE_IGUANA_ALGORITHM(RGAFiducialFilter, clas12::RGAFiducialFilter)

public:
  void Start(hipo::banklist& banks) override;
  void Run  (hipo::banklist& banks) const override;
  void Stop () override {}

  // Programmatic override (takes precedence over YAML). Call before Start().
  void SetStrictness(int strictness);

private:
  // ---- bank indices / presence
  hipo::banklist::size_type b_particle{};
  hipo::banklist::size_type b_config{};
  hipo::banklist::size_type b_calor{};
  hipo::banklist::size_type b_ft{};
  hipo::banklist::size_type b_traj{};
  bool m_have_calor = false;
  bool m_have_ft    = false;
  bool m_have_traj  = false;

  // ---- config knobs
  struct FTParams {
    float rmin;
    float rmax;
    std::vector<std::array<float,3>> holes; // {R,cx,cy}

    // Defaults match the validator overlays.
    FTParams()
      : rmin(8.5f),
        rmax(15.5f),
        holes{
          std::array<float,3>{1.60f, -8.42f,  9.89f},
          std::array<float,3>{1.60f, -9.89f, -5.33f},
          std::array<float,3>{2.30f, -6.15f, -13.00f},
          std::array<float,3>{2.00f,  3.70f,  -6.50f}
        } {}
  };

  struct CVTParams {
    // Layers to test edge on; Java used 1,3,5,7,12
    std::vector<int> edge_layers{1,3,5,7,12};
    // edge minimum (Java used > 0)
    float edge_min = 0.0f;
    // forbidden phi wedges in degrees (open intervals): [lo1,hi1, lo2,hi2, ...]
    // Java used (25,40), (143,158), (265,280)
    std::vector<float> phi_forbidden_deg{25.0f,40.0f, 143.0f,158.0f, 265.0f,280.0f};
  };

  FTParams           u_ft_params{};      // in-use FT params
  CVTParams          u_cvt_params{};     // in-use CVT params
  std::optional<int> u_strictness_user;  // if SetStrictness() used

  // ---- concurrent / per-event state
  mutable std::unique_ptr<ConcurrentParam<int>> o_runnum;
  mutable std::unique_ptr<ConcurrentParam<int>> o_cal_strictness;

  // ---- debug flags
  bool dbg_on     = false;
  bool dbg_ft     = false;
  int  dbg_events = 0;
  static bool EnvOn(const char* name);
  static int  EnvInt(const char* name, int def);

  // ---- lifecycle helpers
  void   Reload(int runnum, concurrent_key_t key) const;
  static bool banklist_has(hipo::banklist& banks, const char* name);

  // ---- filter core (PCAL)
  struct CalHit { int sector=0; float lv=0, lw=0, lu=0; };
  struct CalLayers { std::vector<CalHit> L1; bool has_any=false; };

  static CalLayers CollectCalHitsForTrack(const hipo::bank& cal, int pindex);
  static bool PassCalStrictness(const CalLayers& h, int strictness);

  // ---- filter core (FT)
  bool PassFTFiducial(int track_index, const hipo::bank* ftBank) const;

  // ---- filter core (CVT)
  bool PassCVTFiducial(int track_index, const hipo::bank* trajBank, int strictness) const;

  bool Filter(int track_index,
              const hipo::bank* calBank,
              const hipo::bank* ftBank,
              const hipo::bank* trajBank,
              concurrent_key_t key) const;

  // ---- accessors
  int GetRunNum(concurrent_key_t key) const { return o_runnum->Load(key); }
  int GetCalStrictness(concurrent_key_t key) const { return o_cal_strictness->Load(key); }

  // ---- YAML
  void LoadConfigFromYAML();  // reads from this algorithm's Config.yaml
  void DumpFTParams() const;
  void DumpCVTParams() const;

  // ---- concurrency utils
  concurrent_key_t PrepareEvent(int runnum) const;

  mutable std::mutex m_mutex;
};

} // namespace iguana::clas12