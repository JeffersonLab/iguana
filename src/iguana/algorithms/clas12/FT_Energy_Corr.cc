#include "FT_Energy_Corr.h"
namespace iguana::clas12 {
REGISTER_IGUANA_ALGORITHM(FT_Energy_Corr);

void FT_Energy_Corr::Start(hipo::banklist& banks) {

    //# This is where I would call Fiducial Cuts and Banks in a more complex algorithm.

}

void FT_Energy_Corr::Run(hipo::banklist& banks) const {

    //# This is where I would do the big chunk of calculations in a more complex algorithm.

}


// This function returns an electron 4-vector with the corrected energy for the Forward Tagger. 
// Currently only validated for Fall 2018 outbending data.

TLorentzVector  FT_Energy_Corr::Correct(TLorentzVector x) const{

  Double_t E_new, Px_el, Py_el, Pz_el;
  TLorentzVector el_new;
  E_new = x.E() +  0.0208922 + 0.050158*x.E() - 0.0181107*pow(x.E(),2) + 0.00305671*pow(x.E(),3) - 0.000178235*pow(x.E(),4);

  Px_el = E_new*(x.Px()/x.Rho());
  Py_el = E_new*(x.Py()/x.Rho());
  Pz_el = E_new*(x.Pz()/x.Rho());

  el_new.SetXYZM(Px_el, Py_el, Pz_el, 0.000511);

  return el_new;
}

void ExampleAlgorithm::Stop() {


  }

}
