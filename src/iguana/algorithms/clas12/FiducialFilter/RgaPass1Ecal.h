#pragma once

#include <hipo4/bank.h>
#include <string>
#include <map>

namespace iguana::clas12 {
  class RgaPass1Ecal {

    public:

      enum CutLevel {
        loose,
        medium,
        tight
      };

      /// structure to hold `REC::Calorimeter` data
      struct CalData {
        /// @doxygen_off
        double pcal_lu;
        double pcal_lv;
        double pcal_lw;
        int    pcal_sector;
        double ecin_lu;
        double ecin_lv;
        double ecin_lw;
        int    ecin_sector;
        double ecout_lu;
        double ecout_lv;
        double ecout_lw;
        int    ecout_sector;
        /// @doxygen_on
      };

      RgaPass1Ecal(std::string const& level);

      /// @brief Gets calorimeter data for particles in the event
      /// @param bank the bank to get data from
      /// @returns a map with keys as particle indices (pindex) and values as `CalData` structs
      std::map<int, CalData> GetTrajMap(hipo::bank const& bank) const;

    private:
      CutLevel m_cut_level;
  };
}
