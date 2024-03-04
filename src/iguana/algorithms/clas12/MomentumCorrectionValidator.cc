#include "MomentumCorrectionValidator.h"

namespace iguana::clas12 {

  REGISTER_IGUANA_VALIDATOR(MomentumCorrectionValidator);

  void MomentumCorrectionValidator::Start(hipo::banklist& banks)
  {
    // define the algorithm sequence
    m_algo_seq = std::make_unique<AlgorithmSequence>();
    m_algo_seq->Add("clas12::EventBuilderFilter");
    m_algo_seq->Add("clas12::MomentumCorrection");
    m_algo_seq->SetOption<std::vector<int>>("clas12::EventBuilderFilter", "pids", u_pdg_list);
    m_algo_seq->Start(banks);

    // get bank indices
    b_particle = GetBankIndex(banks, "REC::Particle");

    // set an output file
    auto output_dir = GetOutputDirectory();
    if(output_dir)
      m_output_file = new TFile(TString(output_dir.value()) + "/momentum_corrections.root", "RECREATE");

    // define plots
    for(const auto& pdg : u_pdg_list) {
      TString particle_name  = particle::name.at(particle::PDG(pdg));
      TString particle_title = particle::title.at(particle::PDG(pdg));
      auto after_vs_before = new TH2D(
            "after_vs_before_" + particle_name,
            particle_title + " momentum correction;p_{before} [GeV];p_{after} [GeV]",
            m_num_bins, 0, m_mom_max,
            m_num_bins, 0, m_mom_max);
      u_after_vs_before.insert({pdg, after_vs_before});
    }
  }


  void MomentumCorrectionValidator::Run(hipo::banklist& banks) const
  {
    // get the momenta before
    // FIXME: will need to refactor this once we have HIPO iterators
    auto& particle_bank = GetBank(banks, b_particle, "REC::Particle");
    std::unordered_map<int, double> mom_before;
    for(int row = 0; row < particle_bank.getRows(); row++) {
      double mom = std::hypot(
          particle_bank.getFloat("px", row),
          particle_bank.getFloat("py", row),
          particle_bank.getFloat("pz", row));
      mom_before.insert({row, mom});
    }

    // run the momentum corrections
    m_algo_seq->Run(banks);

    // lock the mutex, so we can mutate plots
    std::scoped_lock<std::mutex> lock(m_mutex);

    // fill the plots
    for(int row = 0; row < particle_bank.getRows(); row++) {

      auto pdg = particle_bank.getInt("pid", row);
      if(pdg == -1)
        continue; // FIXME: will need to refactor this once we have HIPO iterators

      double mom = std::hypot(
          particle_bank.getFloat("px", row),
          particle_bank.getFloat("py", row),
          particle_bank.getFloat("pz", row));
      u_after_vs_before.at(pdg)->Fill(mom_before.at(row), mom);
    }
  }


  void MomentumCorrectionValidator::Stop()
  {
    if(GetOutputDirectory()) {
      m_output_file->Write();
      m_log->Info("Wrote output file {}", m_output_file->GetName());
      m_output_file->Close();
    }
  }

}
