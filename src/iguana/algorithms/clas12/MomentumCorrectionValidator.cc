#include "MomentumCorrectionValidator.h"

namespace iguana::clas12 {

  REGISTER_IGUANA_VALIDATOR(MomentumCorrectionValidator);

  void MomentumCorrectionValidator::Start(hipo::banklist& banks)
  {
    m_algo_seq = std::make_unique<AlgorithmSequence>();
    m_algo_seq->Add("clas12::EventBuilderFilter");
    m_algo_seq->Add("clas12::MomentumCorrection");
    m_algo_seq->SetOption<std::vector<int>>("clas12::EventBuilderFilter", "pids", u_pdg_list);
    m_algo_seq->Start(banks);

    b_particle = GetBankIndex(banks, "REC::Particle");

    for(const auto& pdg : u_pdg_list) {
      TString pdg_title = fmt::format("PDG={}", pdg);
      std::vector<TH2D*> after_vs_before;
      for(const auto& [name, title] : u_mom_strings) {
        after_vs_before.push_back(new TH2D(
            "after_vs_before_" + name,
            title + " momentum correction, " + pdg_title + ";" + title + "^{before} [GeV];" + title + "^{after} [GeV]",
            m_num_bins, 0, m_mom_max,
            m_num_bins, 0, m_mom_max));
      }
      u_after_vs_before.insert({pdg, after_vs_before});
    }
  }


  void MomentumCorrectionValidator::Run(hipo::banklist& banks) const
  {
    auto& particleBank = GetBank(banks, b_particle, "REC::Particle");
    auto particleBankBefore = particleBank; // (use copy ctor)
    m_algo_seq->Run(banks);

    std::lock_guard<std::mutex> guard(m_mutex);

  }


  void MomentumCorrectionValidator::Stop()
  {
  }

}
