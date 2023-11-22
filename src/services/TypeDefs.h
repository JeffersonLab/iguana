#include <memory>
#include <vector>
#include <set>
#include <unordered_map>
#include <variant>

#include <hipo4/bank.h>

namespace iguana {

  /// pointer to a HIPO bank
  using bank_ptr = std::shared_ptr<hipo::bank>;

  /// ordered list of HIPO bank pointers
  using bank_vec_t = std::vector<bank_ptr>;

  /// association between HIPO bank name and its index in a `bank_vec_t`
  using bank_index_cache_t = std::unordered_map<std::string, int>;

  /// option value variant type
  using option_value_t = std::variant<
    int,
    double,
    std::string,
    std::set<int>
  >;

  /// data structure to hold configuration options
  using options_t = std::unordered_map<std::string, option_value_t>;

}
