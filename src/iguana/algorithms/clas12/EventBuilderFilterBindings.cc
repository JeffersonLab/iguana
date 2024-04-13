#include "EventBuilderFilter.h"

namespace iguana::bindings {

  /// @see `iguana::clas12::EventBuilderFilter::Filter`
  /// @param algo the algorithm
  /// @param pid see `iguana::clas12::EventBuilderFilter::Filter`
  /// @returns see `iguana::clas12::EventBuilderFilter::Filter`
  extern "C" bool iguana_clas12_EventBuilderFilter_Filter(void* algo, int const pid)
  {
    return reinterpret_cast<iguana::clas12::EventBuilderFilter*>(algo)->Filter(pid);
  }

}
