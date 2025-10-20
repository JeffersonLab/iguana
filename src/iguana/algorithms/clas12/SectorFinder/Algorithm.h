#pragma once

#include "iguana/algorithms/Algorithm.h"
#include "iguana/algorithms/TypeDefs.h"

namespace iguana::clas12 {

  /// @algo_brief{Find the sector for all rows in `REC::Particle`}
  /// @algo_type_creator
  ///
  /// @begin_doc_config{clas12/SectorFinder}
  /// @config_param{bank_charged | string | if not `default`, use this bank for sector finding of charged particles}
  /// @config_param{bank_neutral | string | if not `default`, use this bank for sector finding of neutral particles}
  /// @end_doc
  ///
  /// If `bank_charged` and/or `bank_neutral` is default, then all of the following banks are needed, in addition to `REC::Particle`:
  ///
  /// - `REC::Track`
  /// - `REC::Calorimeter`
  /// - `REC::Scintillator`
  ///
  /// Otherwise only the bank(s) specified by `bank_charged` and `bank_neutral` is/are needed, if both of them are non-default.
  ///
  /// If the sector cannot be determined, the value `UNKNOWN_SECTOR` will be used instead.
  ///
  /// The action function ::GetStandardSector identifies the sector(s) using these banks in a priority order, whereas
  /// the action function ::GetSector uses a single bank's data.
  /// Note: rows that have been filtered out of `REC::Particle` will still have their sectors determined.
  class SectorFinder : public Algorithm
  {

      DEFINE_IGUANA_ALGORITHM(SectorFinder, clas12::SectorFinder)

    public:

      /// if this algorithm cannot determine the sector, this value will be used
      static int const UNKNOWN_SECTOR = -1;

      void Start(hipo::banklist& banks) override;
      bool Run(hipo::banklist& banks) const override;
      void Stop() override;

      /// @run_function
      /// uses track, calorimeter, and scintillator banks for both charged and neutral particles
      /// @see this algorithm contains multiple run functions, for if you prefer to use other banks
      /// @param [in] particleBank `REC::Particle`
      /// @param [in] trackBank `REC::Track`
      /// @param [in] calBank `REC::Calorimeter`
      /// @param [in] scintBank `REC::Scintillator`
      /// @param [out] resultBank the output `REC::Particle::Sector` bank
      /// @run_function_returns_true
      bool Run(
          hipo::bank const& particleBank,
          hipo::bank const& trackBank,
          hipo::bank const& calBank,
          hipo::bank const& scintBank,
          hipo::bank& resultBank) const
      {
        return RunImpl(&particleBank, &trackBank, &calBank, &scintBank, nullptr, nullptr, &resultBank);
      }

      /// @run_function
      /// uses track, calorimeter, and scintillator banks for charged particles, and a custom bank for neutral particles
      /// @see this algorithm contains multiple run functions, for if you prefer to use other banks
      /// @param [in] particleBank `REC::Particle`
      /// @param [in] trackBank `REC::Track`
      /// @param [in] calBank `REC::Calorimeter`
      /// @param [in] scintBank `REC::Scintillator`
      /// @param [in] userChargedBank custom bank used to obtain charged-particles' sectors
      /// @param [out] resultBank the output `REC::Particle::Sector` bank
      /// @run_function_returns_true
      bool RunWithCustomChargedBank(
          hipo::bank const& particleBank,
          hipo::bank const& trackBank,
          hipo::bank const& calBank,
          hipo::bank const& scintBank,
          hipo::bank const& userChargedBank,
          hipo::bank& resultBank) const
      {
        return RunImpl(&particleBank, &trackBank, &calBank, &scintBank, &userChargedBank, nullptr, &resultBank);
      }

      /// @run_function
      /// uses track, calorimeter, and scintillator banks for neutral particles, and a custom bank for charged particles
      /// @see this algorithm contains multiple run functions, for if you prefer to use other banks
      /// @param [in] particleBank `REC::Particle`
      /// @param [in] trackBank `REC::Track`
      /// @param [in] calBank `REC::Calorimeter`
      /// @param [in] scintBank `REC::Scintillator`
      /// @param [in] userNeutralBank custom bank used to obtain neutral-particles' sectors
      /// @param [out] resultBank the output `REC::Particle::Sector` bank
      /// @run_function_returns_true
      bool RunWithCustomNeutralBank(
          hipo::bank const& particleBank,
          hipo::bank const& trackBank,
          hipo::bank const& calBank,
          hipo::bank const& scintBank,
          hipo::bank const& userNeutralBank,
          hipo::bank& resultBank) const
      {
        return RunImpl(&particleBank, &trackBank, &calBank, &scintBank, nullptr, &userNeutralBank, &resultBank);
      }

      /// @run_function
      /// uses custom banks for both charged and neutral particles
      /// @see this algorithm contains multiple run functions, for if you prefer to use other banks
      /// @param [in] particleBank `REC::Particle`
      /// @param [in] userChargedBank custom bank used to obtain charged-particles' sectors
      /// @param [in] userNeutralBank custom bank used to obtain neutral-particles' sectors
      /// @param [out] resultBank the output `REC::Particle::Sector` bank
      /// @run_function_returns_true
      bool RunWithCustomBanks(
          hipo::bank const& particleBank,
          hipo::bank const& userChargedBank,
          hipo::bank const& userNeutralBank,
          hipo::bank& resultBank) const
      {
        return RunImpl(&particleBank, nullptr, nullptr, nullptr, &userChargedBank, &userNeutralBank, &resultBank);
      }

      /// @action_function{scalar creator} for a given particle with index `pindex_particle`, get its sector from
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
      /// @returns sector for `pindex_particle` in list, `UNKNOWN_SECTOR` if `pindex_particle` not in inputted list
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
      /// @returns sector for `pindex_particle` in lists, `UNKNOWN_SECTOR` if `pindex_particle` not any of the inputted lists
      int GetStandardSector(
          std::vector<int> const& sectors_track,
          std::vector<int> const& pindices_track,
          std::vector<int> const& sectors_cal,
          std::vector<int> const& pindices_cal,
          std::vector<int> const& sectors_scint,
          std::vector<int> const& pindices_scint,
          int const& pindex_particle) const;

      /// @action_function{vector creator} get sectors for all particles, using the standard method
      ///
      /// @overloads_scalar
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

      /// @brief private implementation of the run function, called by public run functions
      /// @param [in] particleBank `REC::Particle`
      /// @param [in] trackBank `REC::Track`
      /// @param [in] calBank `REC::Calorimeter`
      /// @param [in] scintBank `REC::Scintillator`
      /// @param [in] userChargedBank custom bank used to obtain charged-particles' sectors
      /// @param [in] userNeutralBank custom bank used to obtain neutral-particles' sectors
      /// @param [out] resultBank the output `REC::Particle::Sector` bank
      /// @run_function_returns_true
      bool RunImpl(
          hipo::bank const* particleBank,
          hipo::bank const* trackBank,
          hipo::bank const* calBank,
          hipo::bank const* scintBank,
          hipo::bank const* userChargedBank,
          hipo::bank const* userNeutralBank,
          hipo::bank* resultBank) const;

      /// `hipo::banklist` index for the particle bank
      hipo::banklist::size_type b_particle;
      hipo::banklist::size_type b_track;
      hipo::banklist::size_type b_calorimeter;
      hipo::banklist::size_type b_scint;
      hipo::banklist::size_type b_user_charged;
      hipo::banklist::size_type b_user_neutral;
      hipo::banklist::size_type b_result;
      bool userSpecifiedBank_charged{false};
      bool userSpecifiedBank_neutral{false};

      // `b_result` bank item indices
      int i_sector;
      int i_pindex;

      /// Configuration options
      std::string o_bankname_charged;
      std::string o_bankname_neutral;

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
