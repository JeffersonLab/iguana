#pragma once

#include "iguana/algorithms/Validator.h"
#include "iguana/algorithms/TypeDefs.h"
#include <TH2D.h>
#include <TFile.h>
#include <TCanvas.h>
#include <TStyle.h>
#include <Math/Vector4D.h>

namespace iguana::clas12 {

  /// @brief_algo Forward Tagger energy correction
  ///
  /// @begin_doc_algo{Transformer}
  /// @input_banks{RECFT::Particle}
  /// @output_banks{RECFT::Particle}
  /// @end_doc
  class FTEnergyCorrectionValidator : public Validator {

    DEFINE_IGUANA_VALIDATOR(FTEnergyCorrectionValidator, clas12::FTEnergyCorrectionValidator)

    public:

      void Start(hipo::banklist& banks) override;
      void Run(hipo::banklist& banks) const override;
      void Stop() override;

      /// @action_function{scalar transformer}
      /// Transformation function that returns 4-vector of electron with corrected energy for the Forward Tagger.
      /// Currently only validated for Fall 2018 outbending data.
      /// @param px @f$p_x@f$
      /// @param py @f$p_y@f$
      /// @param pz @f$p_z@f$
      /// @param E @f$E@f$
      /// @returns an electron 4-vector with the corrected energy for the Forward Tagger.
      
      vector4_t Transform(
          vector_element_t px,
          vector_element_t py,
          vector_element_t pz,
          vector_element_t E) const;

    private:
      
      // How can I change this FillHistograms function so it works for TH2D histograms? I'm probably being stupid.
      hipo::banklist::size_type b_particle;
      double electron_mass;
      std::vector<int> const u_pdg_list = {particle::PDG::electron, particle::PDG::pi_plus, particle::PDG::pi_minus, particle::PDG::proton};
      TString m_output_file_basename;
      TFile* m_output_file;

      TH2D* h_beforecorr;
      TH2D* h_aftercorr;
  };

}
