#pragma once

#include "iguana/algorithms/TypeDefs.h"
#include "iguana/algorithms/Validator.h"

#include <TFile.h>
#include <TH2.h>
#include <array>
#include <unordered_map>

namespace iguana::clas12 {

  /// @brief Validator for iguana::clas12::RGAFiducialFilter:
  /// For each PID (11, 22) and each layer (PCAL/ECIN/ECOUT), make two 2×3 canvases:
  /// - lv vs lw (six pads = sectors 1..6)
  /// - lv vs lu (six pads = sectors 1..6)
  class RGAFiducialFilterValidator : public Validator
  {
      DEFINE_IGUANA_VALIDATOR(RGAFiducialFilterValidator, clas12::RGAFiducialFilterValidator)

    public:
      void Start(hipo::banklist& banks) override;
      void Run  (hipo::banklist& banks) const override;
      void Stop () override;

    private:
      // banks we read
      hipo::banklist::size_type b_particle{};
      hipo::banklist::size_type b_calor{};

      // pids to plot
      const std::array<int,2> u_pid_list { 11, 22 }; // electrons, photons

      // runtime override for strictness (1..3). Default 1. (used to inform the algorithm)
      int u_strictness_override = 1;

      // Per-PID plot containers
      // layers: 0=PCAL(layer 1), 1=ECIN(layer 4), 2=ECOUT(layer 7)
      // For each layer & sector keep two 2D histos: lv_vs_lw and lv_vs_lu
      struct Layer2D {
        std::array<TH2D*, 7> lv_vs_lw{}; // use indices [1..6]
        std::array<TH2D*, 7> lv_vs_lu{}; // use indices [1..6]
      };
      struct PlotGrid2D { std::array<Layer2D, 3> layer; };
      std::unordered_map<int, PlotGrid2D> u_plots; // pid -> plots

      // output
      TString m_output_file_basename;
      TFile*  m_output_file = nullptr;

      // helpers
      static int         LayerToIndex(int layer);          // 1->0, 4->1, 7->2, else -1
      static const char* LayerName(int layer_idx);         // "PCAL","ECIN","ECOUT"
      static const char* PIDName(int pid);                 // "Electrons"/"Photons"

      void BookPlotsForPID(int pid);
      void DrawSectorGrid2D(int pid, int layer_idx, bool lv_vs_lw);
  };

} // namespace iguana::clas12