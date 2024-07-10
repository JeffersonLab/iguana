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

      /// action function, get sector from list of sectors and pindices from same bank, ordered in same way
      /// nb: this is done instead of finding the pindex in the bank directly to have an action function
      /// @param sectors list of sectors in a bank
      /// @param pindices list of pindices in a bank
      /// @param pindex index in rec::particle bank for which to get sector
      /// @returns sector for pindex in list, -1 if pindex not in inputted list
      int GetSector(std::vector<int> const& sectors, std::vector<int> const& pindices, int const pindex) const;

      /// fill lists of sectors and pindices present in the input bank
      /// @param bank bank from which to get lists of sectors and pindices
      /// @param sectors list to fill with sectors in the bank
      /// @param pindices list to fill with pindices in the bank
      void GetListsSectorPindex(hipo::bank const& bank, std::vector<int>& sectors, std::vector<int>& pindices) const;

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
      int i_pindex;

      /// Configuration options
      std::string o_bankname_charged;
      std::string o_bankname_uncharged;

      //only want sectors from FD detectors
      std::set<int> const listFDDets{6,7,12,15,16,18};
  };

}
