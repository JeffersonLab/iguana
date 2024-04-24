#include "iguana/algorithms/AlgorithmBindings.h"
#include "InclusiveKinematics.h"

namespace iguana::bindings {
  extern "C" {

    /// @see `iguana::InclusiveKinematics::ComputeFromLepton`
    /// @param [in] algo_idx the algorithm index
    /// @param [in] lepton_px, lepton_py, lepton_pz
    /// @param [out] qx, qy, qz, qE, q2, x, y, W, nu
    void iguana_physics_inclusivekinematics_computefromlepton_(
        algo_idx_t* algo_idx,
        vector_element_t* lepton_px,
        vector_element_t* lepton_py,
        vector_element_t* lepton_pz,
        vector_element_t* qx,
        vector_element_t* qy,
        vector_element_t* qz,
        vector_element_t* qE,
        double* Q2,
        double* x,
        double* y,
        double* W,
        double* nu)
    {
      auto out = dynamic_cast<physics::InclusiveKinematics*>(iguana_get_algo_(algo_idx))->ComputeFromLepton(
          *lepton_px,
          *lepton_py,
          *lepton_pz);
      *qx = std::get<0>(out.q);
      *qy = std::get<1>(out.q);
      *qz = std::get<2>(out.q);
      *qE = std::get<3>(out.q);
      *Q2 = out.Q2;
      *x  = out.x;
      *y  = out.y;
      *W  = out.W;
      *nu = out.nu;
    }

  }
}
