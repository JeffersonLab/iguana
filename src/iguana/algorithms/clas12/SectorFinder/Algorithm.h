#pragma once

#include "iguana/algorithms/Algorithm.h"
#include "iguana/algorithms/TypeDefs.h"

namespace iguana::clas12 {

  /// @brief_algo Find the sector for all rows in `REC::Particle`
  ///
  /// @begin_doc_algo{clas12::SectorFinder | Creator}
  /// @input_banks{REC::Particle, REC::Track, REC::Calorimeter, REC::Scintillator}
  /// @output_banks{%REC::Particle::Sector}
  /// @end_doc
  ///
  /// @begin_doc_config{clas12/SectorFinder}
  /// @config_param{bank_charged | string | if not `default`, use this bank for sector finding of charged particles}
  /// @config_param{bank_uncharged | string | if not `default`, use this bank for sector finding of neutral particles}
  /// @end_doc
  ///
  /// If `bank_charged` and/or `bank_uncharged` is default, then all of the following banks are needed, in addition to `REC::Particle`:
  ///
  /// - `REC::Track`
  /// - `REC::Calorimeter`
  /// - `REC::Scintillator`
  ///
  /// Otherwise only the bank(s) specified by `bank_charged` and `bank_uncharged` is/are needed, if both of them are non-default.
  ///
  /// The action function ::GetStandardSector identifies the sector(s) using these banks in a priority order, whereas
  /// the action function ::GetSector uses a single bank's data.
  /// Note: rows that have been filtered out of `REC::Particle` will still have their sectors determined.
  ///
  /// @creator_note
  class SectorFinder : public Algorithm
  {

      DEFINE_IGUANA_ALGORITHM(SectorFinder, clas12::SectorFinder)

    public:

      void Start(hipo::banklist& banks) override;
      void Run(hipo::banklist& banks) const override;
      void Stop() override;

      /// @action_function{vector creator} for a given particle with index `pindex_particle`, get its sector from
      /// a detector bank's list of `sectors` and `pindices` (both must be ordered in the same way)
      ///
      /// @note this is done instead of finding the `pindex` in the bank directly, to have an action function
      ///
      /// @par Example
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
      /// @see ::GetStandardSector, which calls this method for detectors in a priority order
      ///
      /// @param sectors list of sectors in a detector bank
      /// @param pindices list of pindices in a detector bank
      /// @param pindex_particle index in `REC::Particle` bank for which to get sector
      /// @returns sector for `pindex_particle` in list, -1 if `pindex_particle` not in inputted list
      int GetSector(
          std::vector<int> const& sectors,
          std::vector<int> const& pindices,
          int const& pindex_particle) const;

      /// @action_function{scalar creator} for a given particle with index `pindex_particle`, get its sector from
      /// using the standard method
      ///
      /// The following detectors' banks will be searched in order, and once the sector is found for any detector, it is returned:
      ///
      /// - `REC::Track`, using `sectors_track` and `pindices_track`
      /// - `REC::Calorimeter`, using `sectors_cal` and `pindices_cal`
      /// - `REC::Scintillator`, using `sectors_scint` and `pindices_scint`
      ///
      /// @see ::GetSector, which exemplifies using only one bank's lists of `sectors` and `pindices`
      ///
      /// @param sectors_track list of sectors in `REC::Track`
      /// @param pindices_track list of pindices in `REC::Track`
      /// @param sectors_cal list of sectors in `REC::Calorimeter`
      /// @param pindices_cal list of pindices in `REC::Calorimeter`
      /// @param sectors_scint list of sectors in `REC::Scintillator`
      /// @param pindices_scint list of pindices in `REC::Scintillator`
      /// @param pindex_particle index in `REC::Particle` bank for which to get sector
      /// @returns sector for `pindex_particle` in lists, -1 if `pindex_particle` not any of the inputted lists
      int GetStandardSector(
          std::vector<int> const& sectors_track,
          std::vector<int> const& pindices_track,
          std::vector<int> const& sectors_cal,
          std::vector<int> const& pindices_cal,
          std::vector<int> const& sectors_scint,
          std::vector<int> const& pindices_scint,
          int const& pindex_particle) const;

      /// @action_function{vector creator} get sectors for all particles, using
      /// one of the specified detector's list of `sectors` and `pindices`
      ///
      /// @see ::GetSector, which exemplifies using only one bank's lists of `sectors` and `pindices`
      ///
      /// @param sectors_track list of sectors in `REC::Track`
      /// @param pindices_track list of pindices in `REC::Track`
      /// @param sectors_cal list of sectors in `REC::Calorimeter`
      /// @param pindices_cal list of pindices in `REC::Calorimeter`
      /// @param sectors_scint list of sectors in `REC::Scintillator`
      /// @param pindices_scint list of pindices in `REC::Scintillator`
      /// @param pindices_particle the `REC::Particle` list of `pindices`
      /// @returns list of sectors for each particle with `pindex` in `pindices_particle`
      std::vector<int> GetStandardSector(
          std::vector<int> const& sectors_track,
          std::vector<int> const& pindices_track,
          std::vector<int> const& sectors_cal,
          std::vector<int> const& pindices_cal,
          std::vector<int> const& sectors_scint,
          std::vector<int> const& pindices_scint,
          std::vector<int> const& pindices_particle) const;

      /// fill lists of sectors and pindices present in the input bank
      ///
      /// @note this is not an action function, but here for convenience
      ///
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
