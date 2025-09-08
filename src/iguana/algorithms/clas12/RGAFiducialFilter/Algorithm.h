#pragma once

#include "iguana/algorithms/Algorithm.h"
#include "iguana/algorithms/TypeDefs.h"
#include "iguana/services/ConcurrentParam.h"

#include <array>
#include <atomic>
#include <memory>   // std::unique_ptr
#include <mutex>
#include <optional>
#include <string>
#include <vector>

namespace iguana::clas12 {

/// Minimal RGA fiducial filter:
///   - PCAL-only edge cuts on lv & lw with strictness thresholds:
///       s=1 -> {lv,lw} >=  9.0 cm
///       s=2 -> {lv,lw} >= 13.5 cm
///       s=3 -> {lv,lw} >= 18.0 cm
///   - Forward Tagger annulus + circular hole vetoes (from YAML).
///   - Central detector (CVT) fiducial:
///       require edge > edge_min at YAML-selected layers and apply YAML phi-wedge veto
///   - Drift Chamber (DC) fiducial:
///       three regional edge thresholds with inbending/outbending logic (torus-based).
/// All defaults are **required** from Config.yaml. Users may override
/// strictness via SetStrictness(1|2|3) before Start().
///
/// !!!! TO DO: Addition of run-by-run data/MC matching cuts (dead PMTs, etc.)
///
class RGAFiducialFilter : public Algorithm {
  DEFINE_IGUANA_ALGORITHM(RGAFiducialFilter, clas12::RGAFiducialFilter)

public:
  void Start(hipo::banklist& banks) override;
  void Run  (hipo::banklist& banks) const override;
  void Stop () override {}  // no-op

  /// User controlled override (takes precedence over YAML). Call before Start().
  void SetStrictness(int strictness);

private:
  // ---- bank indices and presence flags
  hipo::banklist::size_type b_particle{};
  hipo::banklist::size_type b_config{};
  hipo::banklist::size_type b_calor{};
  hipo::banklist::size_type b_ft{};
  hipo::banklist::size_type b_traj{};
  bool m_have_calor = false;
  bool m_have_ft    = false;
  bool m_have_traj  = false;

  // ---- FT params (values loaded from YAML at Start; no compiled-in defaults)
  struct FTParams {
    float rmin = 0.f;
    float rmax = 0.f;
    std::vector<std::array<float,3>> holes; // {R,cx,cy}
  };
  FTParams           u_ft_params{};      // in-use FT params from YAML
  std::optional<int> u_strictness_user;  // if SetStrictness() used

  // ---- concurrent / per-event state (available for pipelines that need them)
  mutable std::unique_ptr<ConcurrentParam<int>> o_runnum;
  mutable std::unique_ptr<ConcurrentParam<int>> o_cal_strictness;

  // ---- debug flags
  bool dbg_on     = false;
  bool dbg_ft     = false;
  int  dbg_events = 0;
  static bool EnvOn(const char* name);
  static int  EnvInt(const char* name, int def);

  // ---- filter core
  struct CalHit { int sector=0; float lv=0, lw=0, lu=0; };
  struct CalLayers { std::vector<CalHit> L1; bool has_any=false; };

  static CalLayers CollectCalHitsForTrack(const hipo::bank& cal, int pindex);
  static bool PassCalStrictness(const CalLayers& h, int strictness);

  bool PassFTFiducial (int pindex, const hipo::bank* ftBank) const;
  bool PassCVTFiducial(int pindex, const hipo::bank* trajBank, int strictness) const;

  // DC fiducial (detector==6). Uses REC::Particle for pid/px,py,pz and RUN::config for torus.
  bool PassDCFiducial(int pindex,
                      const hipo::bank& particleBank,
                      const hipo::bank& configBank,
                      const hipo::bank* trajBank) const;

  bool Filter(int track_index,
              const hipo::bank& particleBank,
              const hipo::bank& configBank,
              const hipo::bank* calBank,
              const hipo::bank* ftBank,
              const hipo::bank* trajBank,
              concurrent_key_t key) const;

  // ---- accessors (handy for multi-thread pipelines)
  int GetRunNum(concurrent_key_t key) const { return o_runnum->Load(key); }
  int GetCalStrictness(concurrent_key_t key) const { return o_cal_strictness->Load(key); }

  // ---- YAML (required)
  void LoadConfigFromYAML();  // reads from this algorithm's Config.yaml; throws on error
  void DumpFTParams() const;

  // NOTE: No Reload()/PrepareEvent() helpers in this YAML-required version.
};

} // namespace iguana::clas12