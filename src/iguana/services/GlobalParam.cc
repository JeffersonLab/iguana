#include "GlobalParam.h"

namespace iguana {

  // default param values
  GlobalParam<std::string> GlobalConcurrencyModel{"none"};
  GlobalParam<std::string> GlobalRcdbUrl{""};

  // template specializations
  template class GlobalParam<std::string>;
}
