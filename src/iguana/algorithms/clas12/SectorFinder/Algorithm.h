#pragma once

#include "iguana/algorithms/Algorithm.h"
#include "iguana/algorithms/TypeDefs.h"

namespace iguana::clas12 {

  /// @brief_algo Find the sector for all rows in `REC::Particle`
  ///
  /// @begin_doc_algo{clas12::SectorFinder | Creator}
  /// @input_banks{REC::Particle}
  /// @output_banks{%REC::Particle::Sector}
  /// @end_doc
  ///
  /// @begin_doc_config{clas12/SectorFinder}
  /// @config_param{bank_charged | string | if not `default`, use this bank for sector finding of charged particles}
  /// @config_param{bank_uncharged | string | if not `default`, use this bank for sector finding of neutral particles}
  /// @end_doc
  ///
  /// Rows that have been filtered out of `REC::Particle` will still have their sectors determined.
  ///
  /// @creator_note
  class SectorFinder : public Algorithm
  {

      DEFINE_IGUANA_ALGORITHM(SectorFinder, clas12::SectorFinder)

    public:

      void Start(hipo::banklist& banks) override;
      void Run(hipo::banklist& banks) const override;
      void Stop() override;

      /// @action_function{vector creator} for a given particle with index `pindex`, get its sector from a detector bank's list of `sectors` and `pindices` (both must be ordered in the same way);
      /// _nb_: this is done instead of finding the `pindex` in the bank directly, to have an action function
      /// 
      /// **Example:**
      /// ```cpp
      ///
      /// //... Initialise algorithms & banks ... 
      ///
      /// //For each event, do:
      ///
      /// std::vector<int> sectors;
      /// std::vector<int> pindices
      ///
      /// //bank is a hipo::bank object from which we want to get the sectors
      /// //for example the bank object related to REC::Calorimeter
      /// for(auto const& row : bank.getRowList()) {
      ///
      ///     int det=bank.getInt("detector",row);
      ///
      ///     //NB: you should check you read from an FD detector
      ///     // e.g. det 7 is the ECAL (see/use `iguana::DetectorType` enum)
      ///     if(det==7){
      ///       sectors.push_back(bank.getInt("sector", row));
      ///       pindices.push_back(bank.getShort("pindex", row));
      ///     }
      /// }
      ///
      /// //partbank is a hipo::bank object related to REC::Particle
      /// //algo_sector_finder is the iguana::clas12::SectorFinder object
      /// for(auto const& row : partbank.getRowList()) {
      ///     int sector = algo_sector_finder.GetSector(sectors, pindices, row);
      /// }
      /// ```
      ///
      /// @param sectors list of sectors in a detector bank
      /// @param pindices list of pindices in a detector bank
      /// @param pindex index in `REC::Particle` bank for which to get sector
      /// @returns sector for `pindex` in list, -1 if `pindex` not in inputted list
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
      std::set<int> const listFDDets{
        DetectorType::DC,
        DetectorType::ECAL,
        DetectorType::FTOF,
        DetectorType::HTCC,
        DetectorType::LTCC,
        DetectorType::RICH
      };
  };

}
