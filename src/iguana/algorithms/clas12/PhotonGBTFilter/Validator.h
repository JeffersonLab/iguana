#pragma once

#include "iguana/algorithms/Validator.h"
#include "iguana/algorithms/TypeDefs.h"
#include <TH1F.h>
#include <TFile.h>
#include <TCanvas.h>
#include <TStyle.h>
#include <Math/Vector4D.h>
#include <map>

namespace iguana::clas12 {

  /// @brief `iguana::clas12::PhotonGBTFilter` validator
  class PhotonGBTFilterValidator : public Validator
  {

      DEFINE_IGUANA_VALIDATOR(PhotonGBTFilterValidator, clas12::PhotonGBTFilterValidator)

    public:

      void Start(hipo::banklist& banks) override;
      void Run(hipo::banklist& banks) const override;
      void Stop() override;

    private:
      
      void FillHistograms(const std::vector<ROOT::Math::PxPyPzEVector>& photons, int idx) const;
      void InitializeHistograms();
      void ConfigureHistogram(TH1F* hist, int color);
      
      hipo::banklist::size_type b_particle;
      
      std::vector<int> const u_pdg_list = {
          particle::PDG::electron,
          particle::PDG::photon};
      
      TString m_output_file_basename;
      TFile* m_output_file;
      
      
      std::map<int, TH1F*> h_Mgg;
      std::map<int, TH1F*> h_P;
      std::map<int, TH1F*> h_Th;
      std::map<int, TH1F*> h_Phi;

  };

}

