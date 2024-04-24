#include "iguana/algorithms/AlgorithmBindings.h"
#include "EventBuilderFilter.h"

namespace iguana::bindings {
  extern "C" {

    //-------------------------- FIXME: move this to a better place -------------------------
    /// Action function binding. Rules for Fortran compatibility:
    /// - name must be all lowercase and end with an underscore (`_`)
    /// - must be `void`
    /// - parameters must be pointers
    /// - to return a value (or values), mutate the approprate pointers' values
    /// - strings require special treatment: length parameter (must be `int`, and not `int*`) and termination handling
    /// - filter action functions must AND with `out`, to allow function chaining; say
    ///   `*out = *out && _call_action_function_` to avoid the action function call when `! *out`
    //---------------------------------------------------------------------------------------

    /// @see `iguana::clas12::EventBuilderFilter::Filter`
    /// @param [in] algo_idx the algorithm index
    /// @param [in] pid
    /// @param [in,out] out the return value
    void iguana_clas12_eventbuilderfilter_filter_(algo_idx_t* algo_idx, int* pid, bool* out)
    {
      *out = *out && dynamic_cast<clas12::EventBuilderFilter*>(iguana_get_algo_(algo_idx))->Filter(*pid);
    }

  }
}
