#pragma once
/// @file Tools.h

#include <hipo4/bank.h>

namespace iguana::tools {

  /// Get the index of a bank from a `hipo::banklist`.
  /// @note this function is preferred over `hipo::getBanklistIndex`, since it handles the case where there are more than one bank
  /// in the banklist with the same name
  /// @param banks the `hipo::banklist` from which to get the specified bank
  /// @param bank_name the name of the bank
  /// @param variant if 0, the _first_ bank named `bank_name` in `banks` will be returned; if 1, the 2nd such bank will be returned, _etc_.;
  /// note, you can call `Algorithm::GetCreatedBankVariant` to get the created-bank variant number for a specific algorithm
  /// @returns the `hipo::banklist` index of the bank
  /// @see Algorithm::GetCreatedBankVariant to get the variant number for an algorithm's created bank(s), or use Algorithm::GetBankIndex
  hipo::banklist::size_type GetBankIndex(
      hipo::banklist& banks,
      std::string const& bank_name,
      unsigned int const& variant = 0) noexcept(false);

}
