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
    
    // Get CaloMap for the event
    auto calo_map = GetCaloMap(caloBank);
    
    // Loop over each photon in the particleBank to classify it
    for(int row = 0; row < particleBank.getRows(); row++) {
      auto pid    = particleBank.getInt("pid", row);
      if (pid!=22) continue;
      bool b = ClassifyPhoton(particleBank, caloBank, calo_map, row);
    }

  }

  
  bool PhotonGBTFilter::ClassifyPhoton(hipo::bank const &particleBank, hipo::bank const &caloBank, std::map<int, calo_row_data> calo_map, int const row) const
  {
      //Define the variables "m_g" , "m_ch" , "m_nh" 
      //Should not be changed because the model was trained with this specific set of inputs
      int m_g = 3; // Number of neighboring gammas
      int m_ch = 2; // Number of neighboring charged hadrons (protons, pions, kaons)
      int m_nh = 2; // Number of neighboring neutral hadrons (neutrons)
      double gPx=0; // Photon Px
      double gPy=0; // Photon Py
      double gPz=0; // Photon Pz
      double gE=0; // Photon energy
      double gEpcal=0; // Photon pcal energy
      double gTheta=0; // Photon angle
      double gm2u=0; // Photon shower shape
      double gm2v=0; // Photon shower shape

      double R_e;
      double dE_e;

      double R_gamma[m_g]; // Angular distance between calo shower centers
      double dE_gamma[m_g]; // Energy difference
      double Epcal_gamma[m_g]; // Energy deposited in the pcal
      double m2u_gamma[m_g]; // Shower shape variables
      double m2v_gamma[m_g]; // Shower shape variables

      double R_ch[m_ch]; // Angular distance between calo shower centers
      double dE_ch[m_ch]; // Energy difference
      double Epcal_ch[m_ch]; // Energy deposited in the pcal
      double m2u_ch[m_ch]; // Shower shape variables
      double m2v_ch[m_ch]; // Shower shape variables

      double R_nh[m_nh]; // Angular distance between calo shower centers
      double dE_nh[m_nh]; // Energy difference
      double Epcal_nh[m_nh]; // Energy deposited in the pcal
      double m2u_nh[m_nh]; // Shower shape variables
      double m2v_nh[m_nh]; // Shower shape variables

      double num_photons_0_1, num_photons_0_2, num_photons_0_35;

      R_e = 0;
      dE_e = 0;
      //Initialize the arrays
      for (int i=0; i<m_g; ++i) {
        R_gamma[i] = 0;
        dE_gamma[i] = 0;
        Epcal_gamma[i] = 0;
        m2u_gamma[i] = 0;
        m2v_gamma[i] = 0;
      }
      for (int i=0; i<m_ch; ++i){
        R_ch[i] = 0;
        dE_ch[i] = 0;
        Epcal_ch[i] = 0;
        m2u_ch[i] = 0;
        m2v_ch[i] = 0;
      }
      for (int i=0; i<m_nh; ++i){
        R_nh[i] = 0;
        dE_nh[i] = 0;
        Epcal_nh[i] = 0;
        m2u_nh[i] = 0;
        m2v_nh[i] = 0;
      }

      //Set the number of photons within R<0.1, R<0.2, R<0.35
      num_photons_0_1 = 0;
      num_photons_0_2 = 0;
      num_photons_0_35 = 0;
      
      // Set variables native to the photon we are classifying
      gPx = particleBank.getFloat("px",row);
      gPy = particleBank.getFloat("py",row);
      gPz = particleBank.getFloat("pz",row);
      gE = sqrt(gPx*gPx+gPy*gPy+gPz*gPz);
      
      // Loop over particles in the event
      for(int inner_row = 0; inner_row < particleBank.getRows(); inner_row++) {
          auto pid = particleBank.getInt("pid",inner_row);
          auto mass = GetMass(pid);
          std::cout << "The mass of the particle with PID " << pid << " is " << mass << " GeV/c^2" << std::endl;
      }
      
      return true;
  }

    
  std::map<int, PhotonGBTFilter::calo_row_data> PhotonGBTFilter::GetCaloMap(hipo::bank const& bank) const
  {
      std::map<int, PhotonGBTFilter::calo_row_data> calo_map;
      
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
            calo_map[pindex] = calo_row_data();
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
    
  double PhotonGBTFilter::GetMass(int pid) const {
      auto it = mass_map.find(pid);
      if (it != mass_map.end()) {
          return it->second;
      } else {
          return 0.0; // Default mass if pid not found
      }
  }
  void PhotonGBTFilter::Stop()
  {
  }

}
