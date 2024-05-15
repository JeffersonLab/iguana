#include "PhotonGBTFilter.h"

namespace iguana::clas12 {

  REGISTER_IGUANA_ALGORITHM(PhotonGBTFilter);
  // REGISTER_IGUANA_ALGORITHM(PhotonGBTFilter , "clas12::newBank1", "clas12::newBank2"); // if this algorithm creates 2 new banks

  void PhotonGBTFilter::Start(hipo::banklist& banks)
  {
    ParseYAMLConfig();
    b_particle = GetBankIndex(banks, "REC::Particle");
    b_calorimeter = GetBankIndex(banks, "REC::Calorimeter");
    b_config = GetBankIndex(banks, "RUN::config");
  }


    
  void PhotonGBTFilter::Run(hipo::banklist& banks) const
  {
    auto& particleBank = GetBank(banks, b_particle, "REC::Particle");
    auto& caloBank     = GetBank(banks, b_calorimeter, "REC::Calorimeter");
    //auto& configBank   = GetBank(banks, b_config, "RUN::config");
      
    ShowBank(caloBank, Logger::Header("INPUT PARTICLES"));
    
    auto data = GetCaloData(caloBank);
      
    // Iterate over the map and print the data
    for (const auto& entry : data) {
        int pindex = entry.first;
        const auto& calo_data = entry.second;

        std::cout << "Particle index: " << pindex << '\n';
        std::cout << "  pcal_x: " << calo_data.pcal_x << '\n';
        std::cout << "  pcal_y: " << calo_data.pcal_y << '\n';
        std::cout << "  pcal_z: " << calo_data.pcal_z << '\n';
        std::cout << "  pcal_e: " << calo_data.pcal_e << '\n';
        std::cout << "  pcal_m2u: " << calo_data.pcal_m2u << '\n';
        std::cout << "  pcal_m2v: " << calo_data.pcal_m2v << '\n';
        std::cout << "  ecin_x: " << calo_data.ecin_x << '\n';
        std::cout << "  ecin_y: " << calo_data.ecin_y << '\n';
        std::cout << "  ecin_z: " << calo_data.ecin_z << '\n';
        std::cout << "  ecout_x: " << calo_data.ecout_x << '\n';
        std::cout << "  ecout_y: " << calo_data.ecout_y << '\n';
        std::cout << "  ecout_z: " << calo_data.ecout_z << '\n';
        std::cout << "---------------------------" << '\n';
    }
//     for(int row = 0; row < particleBank.getRows(); row++) {
//       auto pid    = particleBank.getInt("pid", row);
//       auto data      = GetCaloData(caloBank,row,0);
//     }

    ShowBank(particleBank, Logger::Header("OUTPUT PARTICLES"));
  }

  
  bool PhotonGBTFilter::Filter(int const pid) const
  {
    return pid > 0;
  }

  std::map<int, PhotonGBTFilter::calo_particle_data> PhotonGBTFilter::GetCaloData(hipo::bank const& bank) const
  {
      std::map<int, PhotonGBTFilter::calo_particle_data> calo_map;
      
      for(int row = 0; row < bank.getRows(); row++){
          auto pindex = bank.getInt("pindex",row);
          auto x      = bank.getFloat("x",row);
          auto y      = bank.getFloat("y",row);
          auto z      = bank.getFloat("z",row);
          auto m2u    = bank.getFloat("m2u",row);
          auto m2v    = bank.getFloat("m2v",row);
          auto layer  = bank.getInt("layer",row);
          auto e      = bank.getFloat("energy",row);
          
          // Ensure an entry exists in the map for the given pindex
          if (calo_map.find(pindex) == calo_map.end()) {
            calo_map[pindex] = calo_particle_data();
          }
          
          switch(layer){
              case 1: // pcal
                calo_map[pindex].pcal_x = x;
                calo_map[pindex].pcal_y = y;
                calo_map[pindex].pcal_z = z;
                calo_map[pindex].pcal_e = e;
                calo_map[pindex].pcal_m2u = m2u;
                calo_map[pindex].pcal_m2v = m2v;
                break;
              case 4: // ecin
                calo_map[pindex].ecin_x = x;
                calo_map[pindex].ecin_y = y;
                calo_map[pindex].ecin_z = z;
                break;
              case 7: // ecout
                calo_map[pindex].ecout_x = x;
                calo_map[pindex].ecout_y = y;
                calo_map[pindex].ecout_z = z;
                break;
          }
      }
      return calo_map;
  }
  void PhotonGBTFilter::Stop()
  {
  }

}
