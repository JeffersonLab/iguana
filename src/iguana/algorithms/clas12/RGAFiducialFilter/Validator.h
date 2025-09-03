#pragma once

#include "iguana/algorithms/TypeDefs.h"
#include "iguana/algorithms/Validator.h"

#include <TFile.h>
#include <TH1.h>
#include <TH2.h>
#include <array>
#include <unordered_map>

namespace iguana::clas12 {

  /// Calorimeter validation:
  ///   For each PID (11=e⁻, 22=γ) and each layer (PCAL/ECIN/ECOUT),
  ///   make two 2×3 canvases (sector grids): lv vs lw, lv vs lu.
  ///
  /// Forward Tagger validation:
  ///   One 1×2 canvas (left: electrons, right: photons) plotting y (cm) vs x (cm).
  class RGAFiducialFilterValidator : public Validator
  {
      DEFINE_IGUANA_VALIDATOR(RGAFiducialFilterValidator, clas12::RGAFiducialFilterValidator)

    public:
      void Start(hipo::banklist& banks) override;
      void Run  (hipo::banklist& banks) const override;
      void Stop () override;

    private:
      // banks we read (indices valid only if corresponding m_have_* is true)
      hipo::banklist::size_type b_particle{};
      hipo::banklist::size_type b_calor{};
      hipo::banklist::size_type b_ft{};
      bool m_have_calor = false;
      bool m_have_ft    = false;

      // pids to plot
      const std::array<int,2> u_pid_list { 11, 22 }; // electrons, photons

      // TH2 containers for calorimeter: [pid][layer 0..2][sector 1..6]
      struct SecH2 { std::array<TH2D*, 7> sec{}; }; // use [1..6]
      struct LayerH2 { SecH2 lv_lw, lv_lu; };
      struct PlotSets { std::array<LayerH2, 3> layer; };

      // one set per PID
      std::unordered_map<int, PlotSets> u_plots2d;

      // FT: per-PID x–y occupancy
      std::unordered_map<int, TH2D*> u_ft_xy;

      // output
      TString m_output_file_basename;
      TFile*  m_output_file = nullptr;

      // helpers
      static int         LayerToIndex(int layer);       // 1->0, 4->1, 7->2, else -1
      static const char* LayerName(int layer_idx);      // "PCAL","ECIN","ECOUT"
      static const char* PIDName(int pid);              // "Electrons"/"Photons"

      void BookPlotsForPID(int pid);
      void DrawSectorGrid2D(int pid, int layer_idx, bool lv_vs_lw); // false -> lv_vs_lu
      void DrawFTCanvas();
  };

} // namespace iguana::clas12