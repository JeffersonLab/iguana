#pragma once

#include "iguana/algorithms/Algorithm.h"

namespace iguana::clas12 {

  /// @brief_algo Find the sector for all rows in `REC::Particle`
  ///
  /// @begin_doc_algo{Creator}
  /// @input_banks{REC::Particle}
  /// @output_banks{%REC::Particle::Sector}
  /// @end_doc
  ///
  /// @begin_doc_config
  /// @config_param{bank | string | if not `default`, use this bank for sector finding}
  /// @end_doc
  class SectorFinder : public Algorithm
  {

      DEFINE_IGUANA_ALGORITHM(SectorFinder, clas12::SectorFinder)

    public:

      void Start(hipo::banklist& banks) override;
      void Run(hipo::banklist& banks) const override;
      void Stop() override;

      /// finds the sector for all rows in REC::Particle
      /// @param banks the list of banks to process
      /// @returns std::vector of sector numbers
      std::vector<int> Find(hipo::banklist& banks) const;

      /// get sector from bank for a given pindex
      /// @param bank bank to get sector from
      /// @param pindex index in bank for which to get sector
      /// @returns sector for pindex in bank
      int GetSector(hipo::bank const& bank, int const pindex) const;

    private:

      /// `hipo::banklist` index for the particle bank
      hipo::banklist::size_type b_particle;
      hipo::banklist::size_type b_calorimeter;
      hipo::banklist::size_type b_track;
      hipo::banklist::size_type b_scint;
      hipo::banklist::size_type b_user;
      bool userSpecifiedBank{false};

      /// Configuration options
      std::string o_bankname;
  };

}
