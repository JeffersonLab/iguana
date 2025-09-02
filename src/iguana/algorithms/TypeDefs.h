/// @file
/// @brief common objects used in algorithms
#pragma once

#include <string>
#include <optional>
#include <unordered_map>

namespace iguana {

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

  //////////////////////////////////////////////////////////////////////////////////

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

    /// @brief get a particle property given a PDG code
    ///
    /// Example:
    /// ```cpp
    /// auto mass = particle::get(particle::mass, particle::PDG::photon); // mass => 0.0
    /// ```
    /// @param property the particle property, such as `particle::name`, `particle::title`, or `particle::mass`
    /// @param pdg_code the `particle::PDG` value
    /// @returns the value of the property, if defined for this `pdg_code`
    template <typename VALUE_TYPE>
    std::optional<VALUE_TYPE> const get(std::unordered_map<PDG,VALUE_TYPE> const& property, PDG const& pdg_code)
    {
      if(auto const& it = property.find(pdg_code); it != property.end())
        return it->second;
      return std::nullopt;
    }

    /// @brief get a particle property given a PDG code
    ///
    /// Example:
    /// ```cpp
    /// auto mass = particle::get(particle::mass, 22); // mass => 0.0
    /// ```
    /// @param property the particle property, such as `particle::name`, `particle::title`, or `particle::mass`
    /// @param pdg_code the `particle::PDG` value
    /// @returns the value of the property, if defined for this `pdg_code`
    template <typename VALUE_TYPE>
    std::optional<VALUE_TYPE> const get(std::unordered_map<PDG,VALUE_TYPE> const& property, int const& pdg_code)
    {
      return get(property, static_cast<particle::PDG>(pdg_code));
    }

    // clang-format on
  }

  //////////////////////////////////////////////////////////////////////////////////

  /// detector IDs; this is a _copy_ of `coatjava`'s `DetectorType` `enum`
  enum DetectorType {
    UNDEFINED = 0,
    BMT       = 1,
    BST       = 2,
    CND       = 3,
    CTOF      = 4,
    CVT       = 5,
    DC        = 6,
    ECAL      = 7,
    FMT       = 8,
    FT        = 9,
    FTCAL     = 10,
    FTHODO    = 11,
    FTOF      = 12,
    FTTRK     = 13,
    HTCC      = 15,
    LTCC      = 16,
    RF        = 17,
    RICH      = 18,
    RTPC      = 19,
    HEL       = 20,
    BAND      = 21,
    RASTER    = 22,
    URWELL    = 23,
    AHDC      = 24,
    ATOF      = 25,
    RECOIL    = 26,
    TARGET    = 100,
    MAGNETS   = 101,
  };

  /// detector layer IDs; this is a _copy_ of `coatjava`'s `DetectorLayer` class
  class DetectorLayer {
    public:
      /// @doxygen_off
      static int const CND_INNER=1;
      static int const CND_MIDDLE=2;
      static int const CND_OUTER=3;

      static int const PCAL_U=1;
      static int const PCAL_V=2;
      static int const PCAL_W=3;
      static int const PCAL_Z=9; // layer number used to define the longitudinal coordinate of the cluster

      static int const EC_INNER_U=4;
      static int const EC_INNER_V=5;
      static int const EC_INNER_W=6;
      static int const EC_INNER_Z=9; // layer number used to define the longitudinal coordinate of the cluster

      static int const EC_OUTER_U=7;
      static int const EC_OUTER_V=8;
      static int const EC_OUTER_W=9;
      static int const EC_OUTER_Z=9; // layer number used to define the longitudinal coordinate of the cluster

      static int const PCAL=PCAL_U;
      static int const EC_INNER=EC_INNER_U;
      static int const EC_OUTER=EC_OUTER_U;

      static int const FTOF1A=1;
      static int const FTOF1B=2;
      static int const FTOF2=3;

      static int const TARGET_CENTER=1;
      static int const TARGET_DOWNSTREAM=2;
      static int const TARGET_UPSTREAM=3;

      static int const FTTRK_MODULE1=1;
      static int const FTTRK_MODULE2=2;
      static int const FTTRK_LAYER1=1;
      static int const FTTRK_LAYER2=2;
      static int const FTTRK_LAYER3=3;
      static int const FTTRK_LAYER4=4;

      static int const RICH_MAPMT=1;
      static int const RICH_AEROGEL_B1=2;
      static int const RICH_AEROGEL_B2=3;
      static int const RICH_AEROGEL_L1=4;
      /// @doxygen_on
  };

}
