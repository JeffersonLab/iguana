#pragma once

#include "iguana/algorithms/TypeDefs.h"
#include "iguana/algorithms/Validator.h"

#include <TFile.h>
#include <TH1.h>
#include <TH2.h>
#include <array>
#include <unordered_map>

namespace iguana::clas12 {

  /// For each PID (11=e⁻, 22=γ) and each layer (PCAL/ECIN/ECOUT),
  /// make two 2×3 canvases (sector grids):
  ///  - lv vs lw
  ///  - lv vs lu
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

      // TH2 containers: [pid][layer 0..2][sector 1..6]
      // layers: 0=PCAL (1), 1=ECIN (4), 2=ECOUT (7)
      struct SecH2 { std::array<TH2D*, 7> sec{}; }; // use [1..6]
      struct LayerH2 { SecH2 lv_lw, lv_lu; };
      struct PlotSets { std::array<LayerH2, 3> layer; };

      // one set per PID
      std::unordered_map<int, PlotSets> u_plots2d;

      // output
      TString m_output_file_basename;
      TFile*  m_output_file = nullptr;

      // helpers
      static int         LayerToIndex(int layer);       // 1->0, 4->1, 7->2, else -1
      static const char* LayerName(int layer_idx);      // "PCAL","ECIN","ECOUT"
      static const char* PIDName(int pid);              // "Electrons"/"Photons"

      void BookPlotsForPID(int pid);
      void DrawSectorGrid2D(int pid, int layer_idx, bool lv_vs_lw); // false -> lv_vs_lu
  };

} // namespace iguana::clas12