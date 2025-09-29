// src/iguana/algorithms/clas12/RGAFiducialFilter/Algorithm.h
#pragma once

#include "iguana/algorithms/Algorithm.h"
#include "iguana/algorithms/TypeDefs.h"

#include <array>
#include <string>
#include <vector>

namespace iguana::clas12 {

  // RGA fiducial filter:
  //   - PCal-only edge cuts on lv & lw with strictness thresholds:
  //       s=1 -> {lv,lw} >=  9.0 cm (bars are width of 4.5cm)
  //       s=2 -> {lv,lw} >= 13.5 cm
  //       s=3 -> {lv,lw} >= 18.0 cm
  //   - Forward Tagger annulus + low efficiency hole vetoes
  //   - Central detector (CVT) fiducial:
  //       require edge > edge_min (default 0) and vetoes on gaps between CVT sectors
  //   - Drift Chamber (DC) fiducial:
  //       three region edge thresholds with separate inbending/outbending track logic
  // All defaults are required from Config.yaml.

  class RGAFiducialFilter : public Algorithm {
    DEFINE_IGUANA_ALGORITHM(RGAFiducialFilter, clas12::RGAFiducialFilter)

  public:
    // public param types 
    struct FTParams {
      float rmin = 0;
      float rmax = 0;
      std::vector<std::array<float,3>> holes; // {R,cx,cy}
    };
    struct CVTParams {
      std::vector<int>    edge_layers;        // e.g. {1,3,5,7,12}
      double              edge_min = 0.0;     // > edge_min
      std::vector<double> phi_forbidden_deg;  // flattened pairs (open intervals)
    };
    struct DCParams {
      // thresholds (cm)
      double theta_small_deg   = 10.0;   // theta boundary for special inbending case
      // inbending, theta < theta_small_deg
      double in_small_e1 = 10.0, in_small_e2 = 10.0, in_small_e3 = 10.0;
      // inbending, theta >= theta_small_deg
      double in_large_e1 = 3.0,  in_large_e2 = 3.0,  in_large_e3 = 10.0;
      // outbending (any theta)
      double out_e1      = 3.0,  out_e2      = 3.0,  out_e3      = 10.0;
    };

    // algorithm API
    void Start(hipo::banklist& banks) override;
    void Run  (hipo::banklist& banks) const override;
    void Stop () override {}

    // ---- Read-only accessors for Validator (single source of truth)
    int                  CalStrictness() const { return m_cal_strictness; }
    const FTParams&      FT()            const { return u_ft_params; }
    const CVTParams&     CVT()           const { return m_cvt; }
    const DCParams&      DC()            const { return m_dc; }

    bool ShouldKeepRow(
      int track_index,
      const hipo::bank& particleBank,
      const hipo::bank& configBank,
      const hipo::bank* calBank,
      const hipo::bank* ftBank,
      const hipo::bank* trajBank) const
    {
      return Filter(track_index, particleBank, configBank, calBank, ftBank, trajBank);
    }

  private:
    // bank indices and presence flags
    hipo::banklist::size_type b_particle{};
    hipo::banklist::size_type b_config{};
    hipo::banklist::size_type b_calor{};
    hipo::banklist::size_type b_ft{};
    hipo::banklist::size_type b_traj{};
    bool m_have_calor = false;
    bool m_have_ft    = false;
    bool m_have_traj  = false;

    // FT params (loaded from YAML)
    FTParams u_ft_params{};            // in-use FT params from YAML

    // core filter functions
    struct CalHit { int sector=0; float lv=0, lw=0, lu=0; };
    struct CalLayers { std::vector<CalHit> L1; bool has_any=false; };

    static CalLayers CollectCalHitsForTrack(const hipo::bank& cal, int pindex);
    static bool PassCalStrictness(const CalLayers& h, int strictness);
    bool PassFTFiducial (int track_index, const hipo::bank* ftBank) const;
    bool PassCVTFiducial(int track_index, const hipo::bank* trajBank) const;
    bool PassDCFiducial(int track_index, const hipo::bank& particleBank,
      const hipo::bank& configBank, const hipo::bank* trajBank) const;
    bool Filter(int track_index, const hipo::bank& particleBank, const hipo::bank& configBank,
        const hipo::bank* calBank, const hipo::bank* ftBank, const hipo::bank* trajBank) const;

    // ---- Config loading (from ConfigFileReader helpers)
    void LoadConfig();

    // CVT/DC params; loaded from Config.yaml (below are defaults which get overwritten)
    int       m_cal_strictness = 1;
    CVTParams m_cvt{};
    DCParams  m_dc{};
  };

} // namespace iguana::clas12