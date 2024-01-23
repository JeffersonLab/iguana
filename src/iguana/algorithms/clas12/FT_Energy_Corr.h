#pragma once

#include "iguana/algorithms/Algorithm.h"
#include LorentzTransformer.h

namespace iguana::clas12 {

  class FT_Energy_Corr : public Algorithm {

    //###############################################################################################
    //# Variables:
    //#      TLorentzVector x: the 4-vector of the electron that needs to have its energy corrected.
    //#      Double_t E_new: the corrected energy.
    //#      Double_t Px_el, Py_el, Pz_el: 3-momentum components of the corrected electron. 
    //#      TLorentzVector el_new: new electron 4-vector with the corrected energy.
    //################################################################################################
    DEFINE_IGUANA_ALGORITHM(FT_Energy_Corr, example::FT_Energy_Corr)

    public:

      void Start(hipo::banklist& banks) override;
      void Run(hipo::banklist& banks) const override;
      void Stop() override;

      // Transformation function that returns 4-vector of electron with corrected energy for the Forward Tagger.
      TLorentzVector Correct(TLorentzVector x) const;
    
    private:
    
      /// Example integer configuration option
      int o_exampleInt;
      /// Example float configuration option
      double o_exampleFloat;

  };

}
