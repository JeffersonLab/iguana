#pragma once

#include "iguana/algorithms/TypeDefs.h"
#include "iguana/algorithms/Validator.h"

#include <TFile.h>
#include <TH1.h>
#include <array>
#include <unordered_map>

namespace iguana::clas12 {

  /// @brief Validator for iguana::clas12::RGAFiducialFilter:
  /// makes two 3x3 canvases (pid 11 and pid 22):
  /// rows = lu, lv, lw; columns = PCal, ECin, ECout; each pad overlays 6 sector histograms.
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

      // runtime override for strictness (1..3). Default 1.
      // You can override via env var IGUANA_RGAFID_STRICTNESS.
      int u_strictness_override = 1;

      // plot container: [layer_idx 0..2][axis_idx 0..2][sector 1..6]
      // layers: 0=PCAL(layer 1), 1=ECIN(layer 4), 2=ECOUT(layer 7)
      // axes:   0=lu, 1=lv, 2=lw
      struct AxisHists { std::array<TH1D*, 7> sec{}; }; // use [1..6]
      struct LayerHists { AxisHists lu, lv, lw; };
      struct PlotGrid   { std::array<LayerHists, 3> layer; };

      // one grid per PID
      std::unordered_map<int, PlotGrid> u_plots;

      // output
      TString m_output_file_basename;
      TFile*  m_output_file = nullptr;

      // helpers
      static int         LayerToIndex(int layer);       // 1->0, 4->1, 7->2, else -1
      static const char* LayerName(int layer_idx);      // "PCAL","ECIN","ECOUT"
      static const char* AxisName (int axis_idx);       // "lu","lv","lw"

      void BookPlotsForPID(int pid);
      void DrawCanvasForPID(int pid);
  };

} // namespace iguana::clas12