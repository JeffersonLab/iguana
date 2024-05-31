#include "Algorithm.h"
#include "iguana/algorithms/Bindings.h"

namespace iguana::bindings::clas12 {
  extern "C" {

  /// @see `iguana::clas12::MomentumCorrection::Transform`
  /// @param [in] algo_idx the algorithm index
  /// @param [in,out] px, py, pz the momentum; it will be corrected
  /// @param [in] sec, pid, torus
  void iguana_clas12_momentumcorrection_transform_(
      algo_idx_t* algo_idx,
      float* px,
      float* py,
      float* pz,
      int* sec,
      int* pid,
      float* torus)
  {
    auto out = dynamic_cast<iguana::clas12::MomentumCorrection*>(iguana_get_algo_(algo_idx))->Transform(vector_element_t(*px), vector_element_t(*py), vector_element_t(*pz), *sec, *pid, *torus);
    *px      = float(std::get<0>(out));
    *py      = float(std::get<1>(out));
    *pz      = float(std::get<2>(out));
  }
  }
}
