#include "LeptonID.h"

#include <cmath>

namespace iguana::clas12 {

  //REGISTER_IGUANA_ALGORITHM(LeptonID); //not using it
  REGISTER_IGUANA_ALGORITHM(LeptonID , "clas12::leptonID"); // this algorithm creates 1 new banks

  void LeptonID::Start(hipo::banklist& banks)
  {
    //Get configuration
    ParseYAMLConfig();
    o_pid        = GetOptionScalar<int>("pid");//Obtain pid from config file
    o_weightfile = GetOptionScalar<std::string>("weightfile");//Obtain weightfile from config file
    //o_exampleDouble = GetOptionScalar<double>("exampleDouble");



    //Get Banks that we are going to use
    b_particle = GetBankIndex(banks, "REC::Particle");
    b_calorimeter = GetBankIndex(banks, "REC::Calorimeter");

    //Create bank to store score (should we add the variables?)
    auto leptonID_schema = CreateBank(
        banks,
        b_leptonID,
        GetClassName(),
        {"pindex/S", "score/D","p/D","theta/D","phi/D","sfpcal/D","sfecin/D","sfecout/D","m2pcal/D","m2ecin/D","m2ecout/D"},
        0xF000,
        1);
    i_pindex   = leptonID_schema.getEntryOrder("pindex");
    i_score = leptonID_schema.getEntryOrder("score");
    i_p = leptonID_schema.getEntryOrder("p");
    i_theta = leptonID_schema.getEntryOrder("theta");
    i_phi = leptonID_schema.getEntryOrder("phi");
    i_sfpcal = leptonID_schema.getEntryOrder("sfpcal");

  }


  void LeptonID::Run(hipo::banklist& banks) const
  {
    auto& particleBank = GetBank(banks, b_particle, "REC::Particle");
    auto& calorimeterBank = GetBank(banks, b_calorimeter, "REC::Calorimeter");

    
    ShowBank(particleBank, Logger::Header("INPUT PARTICLES"));

    auto lepton_pindex = FindLepton(particleBank);
    if(lepton_pindex < 0) {
      ShowBank(leptonID_schema, Logger::Header("CREATED BANK IS EMPTY"));
      return;
    }

    auto lepton_vars=GetLeptonIDVariables(lepton_pindex,particleBank,calorimeterBank);

    result_bank.setRows(1);
    result_bank.putShort(i_pindex, 0, static_cast<int16_t>(lepton_pindex));
    result_bank.putDouble(i_score, 0, lepton_vars.score);

    ShowBank(result_bank, Logger::Header("CREATED BANK"));

    ShowBank(particleBank, Logger::Header("OUTPUT PARTICLES"));
  }


  int FindLepton(hipo::bank const& particle_bank) const{  
    int lepton_pindex= -1;
    for(int row = 0; row < particle_bank.getRows(); row++) {
        auto status = particle_bank.getShort("status", row);
        if(particle_bank.getInt("pid", row) == o_pid && abs(status)>=2000 && abs(status)<4000) {
          lepton_pindex=row;
          break;
        }
      }
      if(lepton_pindex >= 0)
        m_log->Debug("Found lepton: pindex={}", lepton_pindex);
      else
        m_log->Debug("Lepton not found");
      return lepton_pindex;
  }

  LeptonIDVars CalculateScore(int const plepton, hipo::bank const& particle_bank, hipo::bank const& calorimeter_bank) const{

      double px = particle_bank.getFloat("px", plepton);
      double py = particle_bank.getFloat("py", plepton);
      double pz = particle_bank.getFloat("pz", plepton);
      double E = std::sqrt(std::pow(px, 2) + std::pow(py, 2) + std::pow(pz, 2) + std::pow(0.000511, 2));
      ROOT::Math::PxPyPzMVector vec_lepton=(px, py, pz, E);

      LeptonIDVars lepton;

      lepton.P    =vec_lepton.P();
      lepton.Theta=vec_lepton.Theta();
      lepton.Phi  =vec_lepton.Phi();

      m_log->Debug("Variables obtained from particle bank");


      lepton.m2pcal=-1;
      lepton.m2ecin=-1;
      lepton.m2ecout=-1;

      for(int row = 0; row < calorimeter_bank.getRows(); row++) {
          auto pindex = calorimeter_bank.getShort("pindex",row);
          auto layer  = calorimeter_bank.getByte("layer",row);
          auto energy = calorimeter_bank.getFloat("energy",row);
          auto m2u    = calorimeter_bank.getFloat("m2u",row);
          auto m2v    = calorimeter_bank.getFloat("m2v",row);
          auto m2w    = calorimeter_bank.getFloat("m2w",row);

          if(pindex==plepton && layer=1) {
              lepton.SFpcal=energy/vec_lepton.P();
              lepton.m2pcal=(m2u+m2v+m2w)/3;
            }

          if(pindex==plepton && layer==4) {
              lepton.SFecin=energy/vec_lepton.P();
              lepton.m2ecin=(m2u+m2v+m2w)/3;
            }
          if(pindex==plepton && layer==7) {
              lepton.SFecout=energy/vec_lepton.P();
              lepton.m2ecout=(m2u+m2v+m2w)/3;
            }

      }

      
      m_log->Debug("Variables obtained from calorimeter bank");

      //Get TMVA reader
      TMVA::Reader *readerTMVA = new TMVA::Reader( "!Color:!Silent" );
        // Create a set of variables and declare them to the reader
      Float_t P, Theta, Phi, PCAL,ECIN,ECOUT,m2PCAL,m2ECIN,m2ECOUT;

      readerTMVA->AddVariable( "P",&P );
      readerTMVA->AddVariable( "Theta",&Theta);
      readerTMVA->AddVariable( "Phi",&Phi);
      readerTMVA->AddVariable( "SFPCAL",&PCAL);
      readerTMVA->AddVariable( "SFECIN",&ECIN);
      readerTMVA->AddVariable( "SFECOUT",&ECOUT );
      readerTMVA->AddVariable( "m2PCAL",&m2PCAL);
      readerTMVA->AddVariable( "m2ECIN",&m2ECIN);
      readerTMVA->AddVariable( "m2ECOUT",&m2ECOUT);

      m_log->Debug("Add variables to readerTMVA");

	    readerTMVA->BookMVA( "BDT", o_weightfile );

      m_log->Debug("TMVA method booked");

      lepton.score=readerTMVA->EvaluateMVA("BDT");  

      return lepton;
  }


  void LeptonID::Stop()
  {
  }

}
