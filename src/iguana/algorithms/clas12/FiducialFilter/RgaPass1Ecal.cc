#include "RgaPass1Ecal.h"
#include <stdexcept>

namespace iguana::clas12 {

  RgaPass1Ecal::RgaPass1Ecal(std::string const& level)
  {
    if(level == "loose")
      m_cut_level = loose;
    else if(level == "medium")
      m_cut_level = medium;
    else if(level == "tight")
      m_cut_level = tight;
    else
      throw std::runtime_error(std::string("unknown cut level") + level);
  }

  std::map<int, RgaPass1Ecal::CalData> RgaPass1Ecal::GetTrajMap(hipo::bank const& bank) const
  {
    std::map<int, RgaPass1Ecal::CalData> result;
    for(auto const& row : bank.getRowList()){
      auto pindex = bank.getShort("pindex", row);
      auto lu     = bank.getFloat("lu", row);
      auto lv     = bank.getFloat("lv", row);
      auto lw     = bank.getFloat("lw", row);
      auto sector = bank.getByte("sector", row);
      auto layer  = bank.getByte("layer", row);

      if (result.find(pindex) == result.end()) {
        result[pindex] = CalData();
      }

      switch(layer){
        case 1:
          result[pindex].pcal_lu = lu;
          result[pindex].pcal_lv = lv;
          result[pindex].pcal_lw = lw;
          result[pindex].pcal_sector = sector;
          break;
        case 4:
          result[pindex].ecin_lu = lu;
          result[pindex].ecin_lv = lv;
          result[pindex].ecin_lw = lw;
          result[pindex].ecin_sector = sector;
          break;
        case 7:
          result[pindex].ecout_lu = lu;
          result[pindex].ecout_lv = lv;
          result[pindex].ecout_lw = lw;
          result[pindex].ecout_sector = sector;
          break;
      }
    }

    return result;
  }
}
