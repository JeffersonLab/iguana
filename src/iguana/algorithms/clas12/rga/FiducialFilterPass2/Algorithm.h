#pragma once

#include "iguana/algorithms/Algorithm.h"

namespace iguana::clas12::rga {

  /// @algo_brief{Filter the `REC::Particle` bank using subsystem-specific fiducial cuts}
  /// @algo_type_filter
  ///
  /// RGA fiducial filter:
  ///
  /// - PCal-only edge cuts on lv & lw with strictness thresholds
  /// - Forward Tagger annulus + low efficiency hole vetoes
  /// - Central detector (CVT) fiducial:
  ///   - require edge > edge_min (default 0) and vetoes on gaps between CVT sectors
  /// - Drift Chamber (DC) fiducial:
  ///   - three region edge thresholds with separate inbending/outbending track logic
  ///
  /// **References:**
  ///
  /// - https://clas12-docdb.jlab.org/DocDB/0012/001240/001/rga_fiducial_cuts.pdf
  ///
  /// **NOTE:** this algorithm has multiple `Run(hipo::bank bank1, ...)` functions, which
  /// take `hipo::bank` parameters, and some parameters may be optional, since you may
  /// be reading data which lack certain banks. If you use these functions, take a look at all them
  /// to decide which one best suits your use case.
  ///
  /// @begin_doc_config{clas12/rga/FiducialFilterPass2}
  /// @config_param{calorimeter.strictness      | int          | calorimeter cut strictness}
  /// @config_param{forward_tagger.radius       | list[double] | FT allowed radial window (cm)}
  /// @config_param{forward_tagger.holes_flat   | list[double] | FT circular holes (radius, x, y)}
  /// @config_param{cvt.edge_layers             | list[int]    | layers to apply the edge>edge_min test to (all); missing layers are treated as pass}
  /// @config_param{cvt.edge_min                | double       | edge > 0 to ensure tracks inside CVT}
  /// @config_param{cvt.phi_forbidden_deg       | list[double] | forbidden phi wedges in degrees (open intervals)}
  /// @config_param{dc.theta_small_deg          | double       | theta boundary (degrees) for the special inbending case}
  /// @config_param{dc.thresholds_out           | list[double] | outbending thresholds [Region1, Region2, Region3] (cm)}
  /// @config_param{dc.thresholds_in_smallTheta | list[double] | inbending thresholds when theta < theta_small_deg (cm)}
  /// @config_param{dc.thresholds_in_largeTheta | list[double] | inbending thresholds when theta >= theta_small_deg (cm)}
  /// @end_doc
  class FiducialFilterPass2 : public Algorithm {
    DEFINE_IGUANA_ALGORITHM(FiducialFilterPass2, clas12::rga::FiducialFilterPass2)

  private:
    struct FTParams {
      float rmin = 0;
      float rmax = 0;
      std::vector<std::array<float,3>> holes; // {R,cx,cy}
    };
    struct CVTParams {
      std::vector<int>    edge_layers;        // e.g. {1,3,5,7,12}
      double              edge_min = 0.0;     // > edge_min
      std::vector<double> phi_forbidden_deg;  
    };
    struct DCParams {
      // thresholds (cm)
      double theta_small_deg   = 10.0;   // tighter boundary for inbending
      // inbending, theta < theta_small_deg
      double in_small_e1 = 10.0, in_small_e2 = 10.0, in_small_e3 = 10.0;
      // inbending, theta >= theta_small_deg
      double in_large_e1 = 3.0,  in_large_e2 = 3.0,  in_large_e3 = 10.0;
      // outbending (any theta)
      double out_e1      = 3.0,  out_e2      = 3.0,  out_e3      = 10.0;
    };

  public:

    void Start(hipo::banklist& banks) override;
    bool Run  (hipo::banklist& banks) const override;
    void Stop () override {}

    /// @run_function
    /// @param [in,out] particle `REC::Particle` bank, which will be filtered
    /// @param [in] conf `RUN::config` bank
    /// @param [in] cal pointer to `REC::Calorimeter` bank; it is a _pointer_ since it is _optional_ (use `nullptr` for "unused")
    /// @param [in] traj pointer to `REC::Traj` bank; it is a _pointer_ since it is _optional_ (use `nullptr` for "unused")
    /// @param [in] ft pointer to `REC::ForwardTagger` bank; it is a _pointer_ since it is _optional_ (use `nullptr` for "unused")
    /// @returns `false` if all particles are filtered out
    bool Run(
        hipo::bank& particle,
        hipo::bank const& conf,
        hipo::bank const* cal,
        hipo::bank const* traj,
        hipo::bank const* ft) const;

    /// @run_function
    /// @param [in,out] particle `REC::Particle` bank, which will be filtered
    /// @param [in] conf `RUN::config` bank
    /// @param [in] cal `REC::Calorimeter` bank
    /// @param [in] traj `REC::Traj` bank
    /// @returns `false` if all particles are filtered out
    bool Run(
        hipo::bank& particle,
        hipo::bank const& conf,
        hipo::bank const& cal,
        hipo::bank const& traj) const
    {
      return Run(particle, conf, &cal, &traj, nullptr);
    }

    /// @run_function
    /// @param [in,out] particle `REC::Particle` bank, which will be filtered
    /// @param [in] conf `RUN::config` bank
    /// @param [in] cal `REC::Calorimeter` bank
    /// @param [in] traj `REC::Traj` bank
    /// @param [in] ft `REC::ForwardTagger` bank
    /// @returns `false` if all particles are filtered out
    bool Run(
        hipo::bank& particle,
        hipo::bank const& conf,
        hipo::bank const& cal,
        hipo::bank const& traj,
        hipo::bank const& ft) const
    {
      return Run(particle, conf, &cal, &traj, &ft);
    }

    /// @returns calorimeter strictness
    int                  CalStrictness() const { return m_cal_strictness; }
    /// @returns FT configuration parameters
    const FTParams&      FT()            const { return u_ft_params; }
    /// @returns CVT configuration parameters
    const CVTParams&     CVT()           const { return m_cvt; }
    /// @returns DC configuration parameters
    const DCParams&      DC()            const { return m_dc; }

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
    FTParams u_ft_params{};       

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
        const hipo::bank* calBank, const hipo::bank* trajBank, const hipo::bank* ftBank) const;

    // ---- Config loading
    void LoadConfig();

    // CVT/DC params; 
    int       m_cal_strictness = 1;
    CVTParams m_cvt{};
    DCParams  m_dc{};
  };

}
