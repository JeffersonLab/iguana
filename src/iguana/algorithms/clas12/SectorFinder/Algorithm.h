#pragma once

#include "iguana/algorithms/Algorithm.h"

namespace iguana::clas12 {

  /// @brief_algo Find the sector for all rows in `REC::Particle`
  ///
  /// @begin_doc_algo{clas12::SectorFinder | Creator}
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

      /// get sector from bank for a given pindex
      /// @param bank bank to get sector from
      /// @param pindex index in bank for which to get sector
      /// @returns sector for pindex in bank
      int GetSector(hipo::bank const& bank, int const pindex) const;

      /// set sector to output bank for a row in particle bank
      /// useful to avoid duplicating code for charged/uncharged particles
      /// @param banks list of banks used by the algorithm
      /// @param resultBank bank in which we add the sector
      /// @param row row in REC::Particle::Sector to fill
      /// @param userSpecifiedBank whether or not to get sector from user specified bank
      /// @param b_user link to user specified bank
      void setSector(hipo::banklist& banks,hipo::bank& resultBank, int row, bool userSpecifiedBank, hipo::banklist::size_type b_user) const;

    private:

      /// `hipo::banklist` index for the particle bank
      hipo::banklist::size_type b_particle;
      hipo::banklist::size_type b_calorimeter;
      hipo::banklist::size_type b_track;
      hipo::banklist::size_type b_scint;
      hipo::banklist::size_type b_user_charged;
      hipo::banklist::size_type b_user_uncharged;
      hipo::banklist::size_type b_result;
      bool userSpecifiedBank_charged{false};
      bool userSpecifiedBank_uncharged{true};

      // `b_result` bank item indices
      int i_sector;

      /// Configuration options
      std::string o_bankname_charged;
      std::string o_bankname_uncharged;
  };

}
