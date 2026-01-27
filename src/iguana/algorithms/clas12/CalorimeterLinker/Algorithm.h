#pragma once

#include "iguana/algorithms/Algorithm.h"

namespace iguana::clas12 {

  /// @algo_brief{Link particle bank to bank `REC::Calorimeter`}
  /// @algo_type_creator
  ///
  /// This algorithm reads `REC::Calorimeter` and produces a new bank, `REC::Particle::Calorimeter`,
  /// to make it easier to access commonly used `REC::Calorimeter` information for each particle.
  ///
  /// If this algorithm does not provide information you need, ask the maintainers or open a pull request.
  class CalorimeterLinker : public Algorithm
  {

    DEFINE_IGUANA_ALGORITHM(CalorimeterLinker, clas12::CalorimeterLinker)

    private: // hooks
      void StartHook(hipo::banklist& banks) override;
      bool RunHook(hipo::banklist& banks) const override;

    public:

      /// @run_function
      /// @param [in] bank_particle `REC::Particle`
      /// @param [in] bank_calorimeter `REC::Calorimeter`
      /// @param [out] bank_result `REC::Particle::Calorimeter`, which will be created
      /// @run_function_returns_true
      bool Run(
          hipo::bank const& bank_particle,
          hipo::bank const& bank_calorimeter,
          hipo::bank& bank_result) const;

    private:

      /// `hipo::banklist` indices
      hipo::banklist::size_type b_particle;
      hipo::banklist::size_type b_calorimeter;
      hipo::banklist::size_type b_result;

      // `b_result` bank item indices
      int i_pindex;
      int i_pcal_found;
      int i_pcal_sector;
      int i_pcal_lu;
      int i_pcal_lv;
      int i_pcal_lw;
      int i_pcal_energy;
      int i_ecin_found;
      int i_ecin_sector;
      int i_ecin_lu;
      int i_ecin_lv;
      int i_ecin_lw;
      int i_ecin_energy;
      int i_ecout_found;
      int i_ecout_sector;
      int i_ecout_lu;
      int i_ecout_lv;
      int i_ecout_lw;
      int i_ecout_energy;
  };

}
