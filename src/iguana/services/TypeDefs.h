/// @file
/// @brief Type definitions for common objects used in algorithms
#pragma once

#include <string>
#include <unordered_map>

namespace iguana {

  /// concurrent hash key type
  using concurrent_key_t = std::size_t;

  /// Vector element type
  using vector_element_t = double;

  /// 3-momentum type
  struct Momentum3 {
    /// @f$x@f$-component
    vector_element_t px;
    /// @f$y@f$-component
    vector_element_t py;
    /// @f$z@f$-component
    vector_element_t pz;
  };

  /// 4-momentum type
  struct Momentum4 {
    /// @f$x@f$-component
    vector_element_t px;
    /// @f$y@f$-component
    vector_element_t py;
    /// @f$z@f$-component
    vector_element_t pz;
    /// @f$E@f$-component
    vector_element_t E;
  };

  /// Light-weight namespace for particle constants
  namespace particle {
    // clang-format off

    /// PDG codes
    enum PDG {
      electron     = 11,
      photon       = 22,
      proton       = 2212,
      antiproton   = -2212,
      neutron      = 2112,
      antineutron  = -2112,
      pi_plus      = 211,
      pi_minus     = -211,
      kaon_plus    = 321,
      kaon_minus   = -321
    };

    /// Particle names
    const std::unordered_map<PDG, std::string> name{
      { electron, "electron" },
      { photon, "photon" },
      { proton, "proton" },
      { antiproton, "antiproton" },
      { neutron, "neutron" },
      { antineutron, "antineutron" },
      { pi_plus, "pi_plus" },
      { pi_minus, "pi_minus" },
      { kaon_plus, "kaon_plus" },
      { kaon_minus, "kaon_minus" }
    };

    /// Particle titles
    const std::unordered_map<PDG, std::string> title{
      { electron, "e^{-}" },
      { photon, "#gamma" },
      { proton, "p" },
      { antiproton, "#bar{p}" },
      { neutron, "n" },
      { antineutron, "#bar{n}" },
      { pi_plus, "#pi^{+}" },
      { pi_minus, "#pi^{-}" },
      { kaon_plus, "K^{+}" },
      { kaon_minus, "K^{-}" }
    };

    /// Particle mass in GeV
    const std::unordered_map<PDG, double> mass{
      { electron, 0.000511 },
      { photon, 0.0 },
      { proton, 0.938272 },
      { antiproton, 0.938272 },
      { neutron, 0.939565 },
      { antineutron, 0.939565 },
      { pi_plus, 0.139570 },
      { pi_minus, 0.139570 },
      { kaon_plus, 0.493677 },
      { kaon_minus, 0.493677 }
    };

    // clang-format on
  }

}
