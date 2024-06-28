#include "Algorithm.h"

#include <cmath>
#include <Math/Vector4D.h>
#include <TMVA/Reader.h>

namespace iguana::clas12 {

  REGISTER_IGUANA_ALGORITHM(LeptonIDFilter , "clas12::LeptonIDFilter");

  void LeptonIDFilter::Start(hipo::banklist& banks)
  {
    //Get configuration
    ParseYAMLConfig();
    o_pid        = GetOptionScalar<int>("pid");//Obtain pid from config file (+11/-11)
    o_weightfile = GetOptionScalar<std::string>("weightfile");//Obtain weightfile from config file
    o_cut        = GetOptionScalar<double>("cut");



    //Get Banks that we are going to use
    b_particle = GetBankIndex(banks, "REC::Particle");
    b_calorimeter = GetBankIndex(banks, "REC::Calorimeter");


  }


  void LeptonIDFilter::Run(hipo::banklist& banks) const
  {
    auto& particleBank = GetBank(banks, b_particle, "REC::Particle");
    auto& calorimeterBank = GetBank(banks, b_calorimeter, "REC::Calorimeter");
    
    ShowBank(particleBank, Logger::Header("INPUT PARTICLES"));

    //
    particleBank.getMutableRowList().filter([this,&particleBank,&calorimeterBank](auto bank, auto row) {
        auto lepton_pindex = FindLepton(particleBank);
        auto lepton_vars=GetLeptonIDVariables(lepton_pindex,particleBank,calorimeterBank);
        lepton_vars.score=CalculateScore(lepton_vars);

        return Filter(lepton_vars.score);
        });

    // dump the modified bank
    ShowBank(particleBank, Logger::Header("OUTPUT PARTICLES"));
    
  }


  int LeptonIDFilter::FindLepton(hipo::bank const& particle_bank) const{  
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

  LeptonIDVars LeptonIDFilter::GetLeptonIDVariables(int const plepton, hipo::bank const& particle_bank, hipo::bank const& calorimeter_bank) const{

      double px = particle_bank.getFloat("px", plepton);
      double py = particle_bank.getFloat("py", plepton);
      double pz = particle_bank.getFloat("pz", plepton);
      double E = std::sqrt(std::pow(px, 2) + std::pow(py, 2) + std::pow(pz, 2) + std::pow(0.000511, 2));
      ROOT::Math::PxPyPzMVector vec_lepton(px, py, pz, E);

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

          if(pindex==plepton && layer==1) {
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

      return lepton;

  }

  double LeptonIDFilter::CalculateScore(LeptonIDVars lepton_vars) const{

      //Get TMVA reader
      TMVA::Reader *readerTMVA = new TMVA::Reader( "!Color:!Silent" );
        // Create a set of variables and declare them to the reader
      Float_t P, Theta, Phi, PCAL,ECIN,ECOUT,m2PCAL,m2ECIN,m2ECOUT;

      P=lepton_vars.P;
      Theta=lepton_vars.Theta;
      Phi=lepton_vars.Phi;
      PCAL=lepton_vars.SFpcal;
      ECIN=lepton_vars.SFecin;
      ECOUT=lepton_vars.SFecout;
      m2PCAL=lepton_vars.m2pcal;
      m2ECIN=lepton_vars.m2ecin;
      m2ECOUT=lepton_vars.m2ecout;

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

      auto score=readerTMVA->EvaluateMVA("BDT");  

      return score;
  }

  bool LeptonIDFilter::Filter(double score) const{
    if(score>=o_cut)
      return true;
    else
      return false;
  }



  void LeptonIDFilter::Stop()
  {
  }

}
