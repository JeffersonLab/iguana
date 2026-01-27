#pragma once

#include "iguana/algorithms/TypeDefs.h"
#include "iguana/algorithms/Validator.h"
#include <Math/Vector4D.h>
#include <TCanvas.h>
#include <TFile.h>
#include <TH1F.h>
#include <map>

namespace iguana::clas12 {

  /// @brief `iguana::clas12::PhotonGBTFilter` validator
  class PhotonGBTFilterValidator : public Validator
  {

    DEFINE_IGUANA_VALIDATOR(PhotonGBTFilterValidator, clas12::PhotonGBTFilterValidator)

    private: // hooks
      void StartHook(hipo::banklist& banks) override;
      bool RunHook(hipo::banklist& banks) const override;
      void StopHook() override;

    private:

      void FillHistograms(std::vector<ROOT::Math::PxPyPzEVector> const& photons, int idx) const;
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
