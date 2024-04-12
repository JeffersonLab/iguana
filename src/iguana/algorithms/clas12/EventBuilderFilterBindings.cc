#include "EventBuilderFilter.h"

namespace iguana::clas12 {

  extern "C" bool iguana_clas12_EventBuilderFilter_Filter(void* algo, int const pid)
  {
    return reinterpret_cast<EventBuilderFilter*>(algo)->Filter(pid);
  }

}
