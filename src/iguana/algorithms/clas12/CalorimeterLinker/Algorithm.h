#pragma once

#include "iguana/algorithms/Algorithm.h"

namespace iguana::clas12 {

  /// @brief_algo Link particle bank to bank `REC::Calorimeter`
  ///
  /// @begin_doc_algo{clas12::CalorimeterLinker | Creator}
  /// @input_banks{REC::Particle, REC::Calorimeter}
  /// @output_banks{%REC::Particle::Calorimeter}
  /// @end_doc
  ///
  /// This algorithm reads `REC::Calorimeter` and produces a new bank, `REC::Particle::Calorimeter`,
  /// to make it easier to access commonly used `REC::Calorimeter` information for each particle.
  ///
  /// If this algorithm does not provide information you need, ask the maintainers or open a pull request.
  ///
  /// @creator_note
  class CalorimeterLinker : public Algorithm
  {

      DEFINE_IGUANA_ALGORITHM(CalorimeterLinker, clas12::CalorimeterLinker)

    public:

      void Start(hipo::banklist& banks) override;
      void Run(hipo::banklist& banks) const override;
      void Stop() override;

    private:

      /// `hipo::banklist` indices
      hipo::banklist::size_type b_particle;
      hipo::banklist::size_type b_calorimeter;
      hipo::banklist::size_type b_result;

      // `b_result` bank item indices
      int i_pindex;
      int i_pcal_sector;
      int i_pcal_lu;
      int i_pcal_lv;
      int i_pcal_lw;
      int i_ecin_sector;
      int i_ecin_lu;
      int i_ecin_lv;
      int i_ecin_lw;
      int i_ecout_sector;
      int i_ecout_lu;
      int i_ecout_lv;
      int i_ecout_lw;
  };

}
