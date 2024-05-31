#include "Algorithm.h"
#include "iguana/algorithms/Bindings.h"

namespace iguana::bindings {
  extern "C" {

  /// @see `iguana::clas12::ZVertexFilter::Filter`
  /// @param [in] algo_idx the algorithm index
  /// @param [in] vz
  /// @param [in,out] out the return value
  void iguana_clas12_zvertexfilter_filter_(algo_idx_t* algo_idx, float* vz, bool* out)
  {
    *out = *out && dynamic_cast<clas12::ZVertexFilter*>(iguana_get_algo_(algo_idx))->Filter(*vz);
  }
  }
}
