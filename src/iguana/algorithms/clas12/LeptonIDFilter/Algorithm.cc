#include "Algorithm.h"

#include <Math/Vector4D.h>
#include <cmath>

namespace iguana::clas12 {

  REGISTER_IGUANA_ALGORITHM(LeptonIDFilter, "clas12::LeptonIDFilter");

  void LeptonIDFilter::Start(hipo::banklist& banks)
  {
    // Get configuration
    ParseYAMLConfig();
    o_pid                 = GetOptionScalar<int>("pid"); // Obtain pid from config file (+11/-11)
    o_cut                 = GetOptionScalar<double>("cut");
    o_tmva_reader_options = GetOptionScalar<std::string>("tmva_reader_options");
    o_particle_bank       = GetOptionScalar<std::string>("particle_bank");
    o_runnum              = ConcurrentParamFactory::Create<int>();
    o_weightfile          = ConcurrentParamFactory::Create<std::string>();

    // Get Banks that we are going to use
    b_particle    = GetBankIndex(banks, o_particle_bank);
    b_calorimeter = GetBankIndex(banks, "REC::Calorimeter");

    // Initialize the TMVA reader
    readerTMVA = std::make_unique<TMVA::Reader>(LeptonIDVars::names, o_tmva_reader_options);

    // find all the unique weights files in the configuration YAML
    std::set<std::string> weightfile_list;
    for(auto const& node : GetOptionNode({"weightfile"})) {
      for(std::string const particle : {"electron", "positron"}) {
        if(node[particle]) {
          weightfile_list.insert(node[particle].as<std::string>());
        }
      }
    }

    // book the weights files
    // use the name of weightfile as the method tag, for simplicity
    m_log->Debug("Booking weight files:");
    for(auto const& weightfile_name : weightfile_list) {
      auto weightfile_path = GetDataFile(weightfile_name);
      m_log->Debug(" - {}", weightfile_path);
      readerTMVA->BookMVA(weightfile_name, weightfile_path);
    }
  }


  bool LeptonIDFilter::Run(hipo::banklist& banks) const
  {
    return Run(
        GetBank(banks, b_particle, o_particle_bank),
        GetBank(banks, b_calorimeter, "REC::Calorimeter"));
  }


  bool LeptonIDFilter::Run(hipo::bank& particleBank, hipo::bank const& calorimeterBank) const
  {
    // particle bank before filtering
    ShowBank(particleBank, Logger::Header("INPUT PARTICLES"));

    // filter the particle bank
    particleBank.getMutableRowList().filter([this, &particleBank, &calorimeterBank](auto bank, auto row) {
      auto lepton_pindex = FindLepton(particleBank);
      auto lepton_vars   = GetLeptonIDVariables(lepton_pindex, particleBank, calorimeterBank);
      lepton_vars.score  = CalculateScore(lepton_vars);

      return Filter(lepton_vars.score);
    });

    // particle bank after filtering
    ShowBank(particleBank, Logger::Header("OUTPUT PARTICLES"));
    return !particleBank.getRowList().empty();
  }


  int LeptonIDFilter::FindLepton(hipo::bank const& particle_bank) const
  {
    int lepton_pindex = -1;
    for(int row = 0; row < particle_bank.getRows(); row++) {
      auto status = particle_bank.getShort("status", row);
      if(particle_bank.getInt("pid", row) == o_pid && abs(status) >= 2000 && abs(status) < 4000) {
        lepton_pindex = row;
        break;
      }
    }
    if(lepton_pindex >= 0)
      m_log->Debug("Found lepton: pindex={}", lepton_pindex);
    else
      m_log->Debug("Lepton not found");
    return lepton_pindex;
  }


  LeptonIDVars LeptonIDFilter::GetLeptonIDVariables(int const plepton, hipo::bank const& particle_bank, hipo::bank const& calorimeter_bank) const
  {

    double px = particle_bank.getFloat("px", plepton);
    double py = particle_bank.getFloat("py", plepton);
    double pz = particle_bank.getFloat("pz", plepton);
    double E  = std::sqrt(std::pow(px, 2) + std::pow(py, 2) + std::pow(pz, 2) + std::pow(0.000511, 2));
    ROOT::Math::PxPyPzMVector vec_lepton(px, py, pz, E);

    LeptonIDVars lepton;

    lepton.P     = vec_lepton.P();
    lepton.Theta = vec_lepton.Theta();
    lepton.Phi   = vec_lepton.Phi();

    m_log->Debug("Variables obtained from particle bank");


    lepton.m2pcal  = -1;
    lepton.m2ecin  = -1;
    lepton.m2ecout = -1;

    for(int row = 0; row < calorimeter_bank.getRows(); row++) {
      auto pindex = calorimeter_bank.getShort("pindex", row);
      auto layer  = calorimeter_bank.getByte("layer", row);
      auto energy = calorimeter_bank.getFloat("energy", row);
      auto m2u    = calorimeter_bank.getFloat("m2u", row);
      auto m2v    = calorimeter_bank.getFloat("m2v", row);
      auto m2w    = calorimeter_bank.getFloat("m2w", row);

      if(pindex == plepton && layer == 1) {
        lepton.SFpcal = energy / vec_lepton.P();
        lepton.m2pcal = (m2u + m2v + m2w) / 3;
      }

      if(pindex == plepton && layer == 4) {
        lepton.SFecin = energy / vec_lepton.P();
        lepton.m2ecin = (m2u + m2v + m2w) / 3;
      }
      if(pindex == plepton && layer == 7) {
        lepton.SFecout = energy / vec_lepton.P();
        lepton.m2ecout = (m2u + m2v + m2w) / 3;
      }
    }


    m_log->Debug("Variables obtained from calorimeter bank");

    return lepton;
  }


  double LeptonIDFilter::CalculateScore(LeptonIDVars lepton_vars) const
  {
    // Assigning variables from lepton_vars for TMVA method
    return readerTMVA->EvaluateMVA(lepton_vars.GetValues(), "BDT");
  }


  bool LeptonIDFilter::Filter(double score) const
  {
    if(score >= o_cut)
      return true;
    else
      return false;
  }


  void LeptonIDFilter::Stop()
  {
  }

}
