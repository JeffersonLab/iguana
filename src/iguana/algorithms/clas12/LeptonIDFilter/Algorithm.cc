#include "Algorithm.h"
#include "iguana/algorithms/TypeDefs.h"

#include <Math/Vector4D.h>
#include <cmath>

namespace iguana::clas12 {

  REGISTER_IGUANA_ALGORITHM(LeptonIDFilter, "clas12::LeptonIDFilter");

  //////////////////////////////////////////////////////////////////////////////////

  void LeptonIDFilter::ConfigHook()
  {
    // Get configuration
    o_pids                = GetOptionSet<int>({"pids"});
    o_cut                 = GetOptionScalar<double>({"cut"});
    o_tmva_reader_options = GetOptionScalar<std::string>({"tmva_reader_options"});
    o_particle_bank       = GetOptionScalar<std::string>({"particle_bank"});
    o_runnum              = ConcurrentParamFactory::Create<int>();
    o_weightfile_electron = ConcurrentParamFactory::Create<std::string>();
    o_weightfile_positron = ConcurrentParamFactory::Create<std::string>();
  }

  //////////////////////////////////////////////////////////////////////////////////

  void LeptonIDFilter::StartHook(hipo::banklist& banks)
  {
    // Get Banks that we are going to use
    b_particle    = GetBankIndex(banks, o_particle_bank);
    b_calorimeter = GetBankIndex(banks, "REC::Calorimeter");
    b_config      = GetBankIndex(banks, "RUN::config");

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

  //////////////////////////////////////////////////////////////////////////////////

  bool LeptonIDFilter::RunHook(hipo::banklist& banks) const
  {
    return Run(
        GetBank(banks, b_particle, o_particle_bank),
        GetBank(banks, b_calorimeter, "REC::Calorimeter"),
        GetBank(banks, b_config, "RUN::config"));
  }

  //////////////////////////////////////////////////////////////////////////////////

  bool LeptonIDFilter::Run(hipo::bank& particleBank, hipo::bank const& calorimeterBank, hipo::bank const& configBank) const
  {
    // particle bank before filtering
    ShowBank(particleBank, Logger::Header("INPUT PARTICLES"));

    // prepare the event, reloading configuration parameters if the run number changed or is not yet known
    auto key = PrepareEvent(configBank.getInt("run", 0));

    // filter the particle bank
    particleBank.getMutableRowList().filter([this, &calorimeterBank, key](auto bank, auto row) {
      auto pid = bank.getInt("pid", row);
      // check if this is a lepton in `o_pids`
      if(o_pids.find(pid) != o_pids.end()) {
        auto status = bank.getShort("status", row);
        // status cut
        if(std::abs(status) >= 2000 && std::abs(status) < 4000) {
          m_log->Trace("Found lepton: pindex={}", row);
          auto lepton_vars  = GetLeptonIDVariables(row, pid, bank, calorimeterBank);
          lepton_vars.score = CalculateScore(lepton_vars, key);
          return Filter(lepton_vars.score) ? 1 : 0;
        }
        else {
          m_log->Trace("Lepton at pindex={} did not pass status cut", row);
          return 0;
        }
      }
      return 1; // not a lepton in `o_pids`, let it pass the filter
    });

    // particle bank after filtering
    ShowBank(particleBank, Logger::Header("OUTPUT PARTICLES"));
    return !particleBank.getRowList().empty();
  }

  //////////////////////////////////////////////////////////////////////////////////

  concurrent_key_t LeptonIDFilter::PrepareEvent(int const runnum) const
  {
    m_log->Trace("calling PrepareEvent({})", runnum);
    if(o_runnum->NeedsHashing()) {
      std::hash<int> hash_ftn;
      auto hash_key = hash_ftn(runnum);
      if(!o_runnum->HasKey(hash_key))
        Reload(runnum, hash_key);
      return hash_key;
    }
    else {
      if(o_runnum->IsEmpty() || o_runnum->Load(0) != runnum)
        Reload(runnum, 0);
      return 0;
    }
  }

  //////////////////////////////////////////////////////////////////////////////////

  void LeptonIDFilter::Reload(int const runnum, concurrent_key_t key) const
  {
    std::lock_guard<std::mutex> const lock(m_mutex); // NOTE: be sure to lock successive `ConcurrentParam::Save` calls !!!
    m_log->Trace("-> calling Reload({}, {})", runnum, key);
    o_runnum->Save(runnum, key);
    o_weightfile_electron->Save(GetOptionScalar<std::string>({"weightfile", GetConfig()->InRange("runs", runnum), "electron"}), key);
    o_weightfile_positron->Save(GetOptionScalar<std::string>({"weightfile", GetConfig()->InRange("runs", runnum), "positron"}), key);
  }

  //////////////////////////////////////////////////////////////////////////////////

  LeptonIDVars LeptonIDFilter::GetLeptonIDVariables(int const plepton, int const pdg, hipo::bank const& particle_bank, hipo::bank const& calorimeter_bank) const
  {

    double px = particle_bank.getFloat("px", plepton);
    double py = particle_bank.getFloat("py", plepton);
    double pz = particle_bank.getFloat("pz", plepton);
    auto mass = particle::get(particle::mass, pdg).value();
    double E  = std::sqrt(std::pow(px, 2) + std::pow(py, 2) + std::pow(pz, 2) + std::pow(mass, 2));
    ROOT::Math::PxPyPzMVector vec_lepton(px, py, pz, E);

    LeptonIDVars lepton;

    lepton.pid   = pdg;
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

  //////////////////////////////////////////////////////////////////////////////////

  double LeptonIDFilter::CalculateScore(LeptonIDVars lepton_vars, concurrent_key_t const key) const
  {
    // Assigning variables from lepton_vars for TMVA method
    std::string weightsfile;
    switch(lepton_vars.pid) {
    case particle::PDG::electron:
      weightsfile = o_weightfile_electron->Load(key);
      break;
    case particle::PDG::positron:
      weightsfile = o_weightfile_positron->Load(key);
      break;
    default:
      throw std::runtime_error(fmt::format("unknown lepton PDG code {}", lepton_vars.pid));
    }
    return readerTMVA->EvaluateMVA(lepton_vars.GetValues(), weightsfile);
  }

  //////////////////////////////////////////////////////////////////////////////////

  bool LeptonIDFilter::Filter(double score) const
  {
    if(score >= o_cut)
      return true;
    else
      return false;
  }

}
