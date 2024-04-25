#include "iguana/algorithms/AlgorithmBindings.h"
#include "InclusiveKinematics.h"

namespace iguana::bindings::physics {
  extern "C" {

    /// @see `iguana::physics::InclusiveKinematics::ComputeFromLepton`
    /// @param [in] algo_idx the algorithm index
    /// @param [in] lepton_px, lepton_py, lepton_pz scattered lepton momentum
    /// @param [out] qx, qy, qz, qE, Q2, x, y, W, nu inclusive kinematics
    void iguana_physics_inclusivekinematics_computefromlepton_(
        algo_idx_t* algo_idx,
        double* lepton_px,
        double* lepton_py,
        double* lepton_pz,
        double* qx,
        double* qy,
        double* qz,
        double* qE,
        double* Q2,
        double* x,
        double* y,
        double* W,
        double* nu)
    {
      auto out = dynamic_cast<iguana::physics::InclusiveKinematics*>(iguana_get_algo_(algo_idx))->ComputeFromLepton(
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
