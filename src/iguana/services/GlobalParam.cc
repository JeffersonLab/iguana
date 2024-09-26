#include "GlobalParam.h"

namespace iguana {

  // default param values
  GlobalParam<std::string> GlobalConcurrencyModel{"none"};

  // template specializations
  template class GlobalParam<std::string>;
}
