#pragma once

#include "Algorithm.h"
#include "iguana/algorithms/TypeDefs.h"
#include "iguana/algorithms/Validator.h"
#include "iguana/algorithms/clas12/CalorimeterLinker/Algorithm.h"
#include "iguana/algorithms/clas12/EventBuilderFilter/Algorithm.h"
#include "iguana/algorithms/clas12/TrajLinker/Algorithm.h"

#include <TCanvas.h>
#include <TFile.h>
#include <TH2.h>

namespace iguana::clas12::rga {

  /// @brief `iguana::clas12::rga::FiducialFilterPass1` validator
  class FiducialFilterPass1Validator : public Validator
  {

    DEFINE_IGUANA_VALIDATOR(FiducialFilterPass1Validator, clas12::rga::FiducialFilterPass1Validator)

    private: // hooks
      void StartHook(hipo::banklist& banks) override;
      bool RunHook(hipo::banklist& banks) const override;
      void StopHook() override;

    private:

      iguana::clas12::EventBuilderFilter m_algo_eb;
      iguana::clas12::TrajLinker m_algo_traj;
      iguana::clas12::CalorimeterLinker m_algo_cal;
      iguana::clas12::rga::FiducialFilterPass1 m_algo_fidu;

      hipo::banklist::size_type b_particle;
      hipo::banklist::size_type b_traj;
      hipo::banklist::size_type b_cal;

      double const DC1xleft   = -200;
      double const DC1xright  = 200;
      double const DC1ybottom = -200;
      double const DC1ytop    = 200;
      double const DC2xleft   = -200;
      double const DC2xright  = 200;
      double const DC2ybottom = -200;
      double const DC2ytop    = 200;
      double const DC3xleft   = -200;
      double const DC3xright  = 200;
      double const DC3ybottom = -200;
      double const DC3ytop    = 200;

      std::vector<int> const u_pdg_list = {
          particle::PDG::electron,
          particle::PDG::pi_plus,
          particle::PDG::pi_minus,
          particle::PDG::proton};

      TString m_output_file_basename;
      TFile* m_output_file;
      mutable std::unordered_map<int, TH2D*> u_DC1_before;
      mutable std::unordered_map<int, TH2D*> u_DC2_before;
      mutable std::unordered_map<int, TH2D*> u_DC3_before;

      mutable std::unordered_map<int, TH2D*> u_DC1_after;
      mutable std::unordered_map<int, TH2D*> u_DC2_after;
      mutable std::unordered_map<int, TH2D*> u_DC3_after;
  };

}
