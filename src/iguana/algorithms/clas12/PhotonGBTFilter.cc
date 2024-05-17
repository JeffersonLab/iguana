#include "PhotonGBTFilter.h"

namespace iguana::clas12 {

  REGISTER_IGUANA_ALGORITHM(PhotonGBTFilter);

  void PhotonGBTFilter::Start(hipo::banklist& banks)
  {
    ParseYAMLConfig();
      
    b_particle = GetBankIndex(banks, "REC::Particle");
    b_calorimeter = GetBankIndex(banks, "REC::Calorimeter");
    b_config = GetBankIndex(banks, "RUN::config");
    
    o_pass      = GetCachedOption<int>("pass").value_or(0);
    o_threshold = GetCachedOption<double>("threshold").value_or(0);
    
  }


    
  void PhotonGBTFilter::Run(hipo::banklist& banks) const
  {
      
    auto& particleBank = GetBank(banks, b_particle, "REC::Particle");
    auto& caloBank     = GetBank(banks, b_calorimeter, "REC::Calorimeter");
    auto& configBank = GetBank(banks,b_config,"RUN::config");
    int runnum = configBank.getInt("run",0);
       
    // Get CaloMap for the event
    auto calo_map = GetCaloMap(caloBank);
    
    // dump the bank
    ShowBank(particleBank, Logger::Header("INPUT PARTICLES"));
      
    // Loop over each photon in the particleBank to classify it
    for(int row = 0; row < particleBank.getRows(); row++) {
      auto pid    = particleBank.getInt("pid", row);
      if (pid!=22) continue;
      auto accept = Filter(particleBank, caloBank, calo_map, row, runnum);
      if(!accept)
        MaskRow(particleBank, row);
    }

    // dump the modified bank
    ShowBank(particleBank, Logger::Header("OUTPUT PARTICLES"));
      
  }

  bool PhotonGBTFilter::PidPurityPhotonFilter(float const E, float const Epcal, float const theta) const
  {
      // Apply standard pid cuts on the photon, compatible to how the models were trained
      // 1. Minimum photon energy cut of 200 MeV
      // 2. Photon must have deposited energy in the PCal
      // 3. Photon must be in the Forward Detector
      if( E<0.2 || Epcal <= 0 || !(ForwardDetectorFilter(theta))) return false;
      return true;
  }

  bool PhotonGBTFilter::ForwardDetectorFilter(float const theta) const
  {
      // Apply forward detector cut
      if( theta*180/3.14159265<5 || theta*180/3.14159265>35 ) return false;
      return true;
  }
    
  bool PhotonGBTFilter::Filter(hipo::bank const &particleBank, hipo::bank const &caloBank, std::map<int, calo_row_data> calo_map, int const row, int const runnum) const
  {

      // Set variables native to the photon we are classifying
      double gPx = particleBank.getFloat("px",row);
      double gPy = particleBank.getFloat("py",row);
      double gPz = particleBank.getFloat("pz",row);
      
      // Set ML features intrinsic to the photon of interest
      double gE = sqrt(gPx*gPx+gPy*gPy+gPz*gPz);
      double gTheta = acos(gPz / gE);
      double gEpcal = calo_map[row].pcal_e;
      double gm2u = calo_map[row].pcal_m2u;
      double gm2v = calo_map[row].pcal_m2v;
      
      // Apply PID purity cuts on the photon
      // If they do not pass, then these photons are incompatible with the trained GBT model
      if( PidPurityPhotonFilter(gE,gEpcal,gTheta)==false ) return false;
      
      //Define the variables "m_g" , "m_ch" , "m_nh" 
      //Should not be changed because the model was trained with this specific set of inputs
      int m_g = 3; // Number of neighboring gammas
      int m_ch = 2; // Number of neighboring charged hadrons (protons, pions, kaons)
      int m_nh = 2; // Number of neighboring neutral hadrons (neutrons)

      double R_e = 0;
      double dE_e = 0;

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
      
      
      // Get 3-vector that points to the photon of interest's location in the calorimeter
      ROOT::Math::XYZVector vPOI = GetParticleCaloVector(calo_map, row);
      
      // Build nearest neighbor event structure
      // Loop over particles in the event
      for(int inner_row = 0; inner_row < particleBank.getRows(); inner_row++) {
          // Skip over the particle if it is photon we are trying to classify
          if (inner_row == row) continue;
          
          // Check if the key exists in the calo_map
          // This skips over REC::Particle entries without a REC::Calorimeter entry
          if (calo_map.find(inner_row) == calo_map.end()) continue;
          
          auto pid = particleBank.getInt("pid",inner_row);
          auto mass = GetMass(pid);
          // Skip over particle if its mass was undefined
          if (mass == -1.0) continue;
          auto px = particleBank.getFloat("px",inner_row);
          auto py = particleBank.getFloat("py",inner_row);
          auto pz = particleBank.getFloat("pz",inner_row);
          auto p  = sqrt(px*px+py*py+pz*pz);
          auto E  = sqrt(p*p+mass*mass);
          auto th = acos(pz/p);
          // Skip over particle if it is not in the forward detector (necessary for model compatibility)
          if (ForwardDetectorFilter(th)==false) continue;
          
          // Get 3-vector that points to the neighboring particle's location in the calorimeter
          ROOT::Math::XYZVector vPART = GetParticleCaloVector(calo_map, inner_row);
          
          // Get angular distance between photon of interest and particle
          double R = ROOT::Math::VectorUtil::Angle(vPOI, vPART);
          
          // Get 'type' of particle for the neighbor
          // 0 --> photon
          // 1 --> electron
          // 2 --> charged hadron
          // 3 --> neutral hadron
          int part_type = GetParticleType(pid);
          
          // Logic for filling nearest neighbor variables
          if(part_type==0) {//photon
              
              // Apply Photon Purity Cuts to ensure this neighbor can be used in classification
              if ( PidPurityPhotonFilter(E,calo_map[inner_row].pcal_e,th)==false ) continue;
              
              if (R < 0.1) num_photons_0_1++;
              if (R < 0.2) num_photons_0_2++;
              if (R < 0.35) num_photons_0_35++;
              
              for (int i=0; i<m_g; ++i) {
                  if (R < R_gamma[i] || R_gamma[i] == 0) {
                    int j = m_g - 1;
                    while (j > i) {
                        R_gamma[j] = R_gamma[j - 1];
                        dE_gamma[j] = dE_gamma[j - 1];
                        Epcal_gamma[j] = Epcal_gamma[j - 1];
                        m2u_gamma[j] = m2u_gamma[j - 1];
                        m2v_gamma[j] = m2v_gamma[j - 1];
                        j--;
                    }
                    R_gamma[i] = R;
                    dE_gamma[i] = gE - E;
                    Epcal_gamma[i] = calo_map[inner_row].pcal_e;
                    m2u_gamma[i] = calo_map[inner_row].pcal_m2u;
                    m2v_gamma[i] = calo_map[inner_row].pcal_m2v;
                    break;
                  }
              }
          }
          else if(part_type==1){//electron
              if(R<R_e || R_e==0){
                  R_e = R;
                  dE_e = gE - E;
              }
          }
          else if(part_type==2){//charged hadron
              for (int i=0; i<m_ch; ++i) {
                  if (R < R_ch[i] || R_ch[i] == 0) {
                    int j = m_ch - 1;
                    while (j > i) {
                        R_ch[j] = R_ch[j - 1];
                        dE_ch[j] = dE_ch[j - 1];
                        Epcal_ch[j] = Epcal_ch[j - 1];
                        m2u_ch[j] = m2u_ch[j - 1];
                        m2v_ch[j] = m2v_ch[j - 1];
                        j--;
                    }
                    R_ch[i] = R;
                    dE_ch[i] = gE- E;
                    Epcal_ch[i] = calo_map[inner_row].pcal_e;
                    m2u_ch[i] = calo_map[inner_row].pcal_m2u;
                    m2v_ch[i] = calo_map[inner_row].pcal_m2v;
                    break;
                  }
                }
          }
          else if(part_type==3){//neutral hadron
              for (int i=0; i<m_nh; ++i) {
                  if (R < R_nh[i] || R_nh[i] == 0) {
                    int j = m_nh - 1;
                    while (j > i) {
                        R_nh[j] = R_nh[j - 1];
                        dE_nh[j] = dE_nh[j - 1];
                        Epcal_nh[j] = Epcal_nh[j - 1];
                        m2u_nh[j] = m2u_nh[j - 1];
                        m2v_nh[j] = m2v_nh[j - 1];
                        j--;
                    }
                    R_nh[i] = R;
                    dE_nh[i] = gE - E;
                    Epcal_nh[i] = calo_map[inner_row].pcal_e;
                    m2u_nh[i] = calo_map[inner_row].pcal_m2u;
                    m2v_nh[i] = calo_map[inner_row].pcal_m2v;
                    break;
                  }
                }
          }
          else{//unrecognized OR uncompatible particle type for the trained model
              continue;
          }
      }
      
      // Create and populate input_data vector for the ML model
      std::vector<float> input_data = {
          static_cast<float>(gE), static_cast<float>(gEpcal), static_cast<float>(gTheta),
          static_cast<float>(gm2u), static_cast<float>(gm2v), static_cast<float>(R_e),
          static_cast<float>(dE_e)
      };

      for (int i = 0; i < m_g; ++i) {
          input_data.push_back(static_cast<float>(R_gamma[i]));
          input_data.push_back(static_cast<float>(dE_gamma[i]));
          input_data.push_back(static_cast<float>(Epcal_gamma[i]));
          input_data.push_back(static_cast<float>(m2u_gamma[i]));
          input_data.push_back(static_cast<float>(m2v_gamma[i]));
      }

      for (int i = 0; i < m_ch; ++i) {
          input_data.push_back(static_cast<float>(R_ch[i]));
          input_data.push_back(static_cast<float>(dE_ch[i]));
          input_data.push_back(static_cast<float>(Epcal_ch[i]));
          input_data.push_back(static_cast<float>(m2u_ch[i]));
          input_data.push_back(static_cast<float>(m2v_ch[i]));
      }

      for (int i = 0; i < m_nh; ++i) {
          input_data.push_back(static_cast<float>(R_nh[i]));
          input_data.push_back(static_cast<float>(dE_nh[i]));
          input_data.push_back(static_cast<float>(Epcal_nh[i]));
          input_data.push_back(static_cast<float>(m2u_nh[i]));
          input_data.push_back(static_cast<float>(m2v_nh[i]));
      }

      input_data.push_back(static_cast<float>(num_photons_0_1));
      input_data.push_back(static_cast<float>(num_photons_0_2));
      input_data.push_back(static_cast<float>(num_photons_0_35));

      return ClassifyPhoton(input_data, runnum);
  }

  bool PhotonGBTFilter::ClassifyPhoton(std::vector<float> const &input_data, int const runnum) const
  {

      double sigmoid_x = 0.0; // Input to sigmoid
      
      // Assign the correct function based on runnum
      if (runnum>=5032&&runnum<=5332) { // Fall2018 RGA Inbending
          sigmoid_x = ApplyCatboostModel_RGA_inbending(input_data); 
      } else if (runnum>=5333&&runnum<=5666) { // Fall2018 RGA Outbending
          sigmoid_x = ApplyCatboostModel_RGA_outbending(input_data); 
      } else if (runnum>=6616&&runnum<=6783) { // Spring2019 RGA Inbending
          sigmoid_x = ApplyCatboostModel_RGA_inbending(input_data); 
      } else if (runnum>=6156&&runnum<=6603) { // Spring2019 RGB Inbending
          sigmoid_x = ApplyCatboostModel_RGA_inbending(input_data); 
      } else if (runnum>=11093&&runnum<=11283) { // Fall2019 RGB Outbending
          sigmoid_x = ApplyCatboostModel_RGA_outbending(input_data); 
      } else if (runnum>=11284&&runnum<=11300) { // Fall2019 RGB BAND Inbending
          sigmoid_x = ApplyCatboostModel_RGA_inbending(input_data); 
      } else if (runnum>=11323&&runnum<=11571) { // Spring2020 RGB Inbending
          sigmoid_x = ApplyCatboostModel_RGA_inbending(input_data); 
      } else if (runnum>=16042&&runnum<=16772) { // Summer2022 RGC Inbending
          sigmoid_x = ApplyCatboostModel_RGC_Summer2022(input_data); 
      } else { // Unknown, defaulting to RG-A inbending
          m_log->Warn("Run Number {} has no matching PhotonGBT model...Defaulting to RGA inbending...",runnum);
          sigmoid_x = ApplyCatboostModel_RGA_inbending(input_data); 
      }
      
      double prediction = 1-1/(1+exp(-sigmoid_x));               // Calculate predictive value for "signal"
                                                                 // [0,1] --> Closer to 1 == Photon is Signal

      return (prediction>o_threshold);
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

  ROOT::Math::XYZVector PhotonGBTFilter::GetParticleCaloVector(std::map<int, calo_row_data> const &calo_map, int const row) const {
      // Determine the 3-vector location of where the photon of interest's calo deposition is
      // First we check the pcal coords, then ecin, then ecout
      ROOT::Math::XYZVector v;
      if (calo_map.at(row).pcal_x == -999) {
          if (calo_map.at(row).ecin_x == -999) {
              v.SetXYZ(calo_map.at(row).ecout_x, calo_map.at(row).ecout_y, calo_map.at(row).ecout_z);
          } else {
              v.SetXYZ(calo_map.at(row).ecin_x, calo_map.at(row).ecin_y, calo_map.at(row).ecin_z);
          }
      } else {
          v.SetXYZ(calo_map.at(row).pcal_x, calo_map.at(row).pcal_y, calo_map.at(row).pcal_z);
      }
      return v;
  }

  double PhotonGBTFilter::GetMass(int pid) const {
  auto it = particle::mass.find(static_cast<particle::PDG>(pid));
  if (it != particle::mass.end()) {
    return it->second;
  } else {
    return -1.0; // Default mass if pid not found
  }
}
    

  int PhotonGBTFilter::GetParticleType(int pid) const {
    auto it = type_map.find(pid);
    if (it != type_map.end()) {
        return it->second;
    } else {
        return -1; // Default type if pid not found
    }
  }
  void PhotonGBTFilter::Stop()
  {
  }

}
