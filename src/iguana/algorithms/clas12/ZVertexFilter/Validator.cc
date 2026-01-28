#include "Validator.h"

#include <TCanvas.h>
#include <TLegend.h>

namespace iguana::clas12 {

  REGISTER_IGUANA_VALIDATOR(ZVertexFilterValidator);

  void ZVertexFilterValidator::StartHook(hipo::banklist& banks)
  {
    // define the algorithm sequence
    m_algo_seq = std::make_unique<AlgorithmSequence>();
    m_algo_seq->Add("clas12::ZVertexFilter");
    m_algo_seq->Start(banks);

    // get bank indices
    b_particle = GetBankIndex(banks, "REC::Particle");

    // set an output file
    auto output_dir = GetOutputDirectory();
    if(output_dir) {
      m_output_file_basename = output_dir.value() + "/zvertex_filter";
      m_output_file          = new TFile(m_output_file_basename + ".root", "RECREATE");
    }

    // define plots
    for(auto const& pdg : u_pdg_list) {
      std::vector<TH1D*> zvertexplots;
      TString particle_name  = particle::name.at(particle::PDG(pdg));
      TString particle_title = particle::title.at(particle::PDG(pdg));
      for(int i = 0; i < 2; i++) {
        TString beforeafter_name = "before";
        if(i == 1) {
          beforeafter_name = "after";
        }

        zvertexplots.push_back(new TH1D(
            "zvertexplots_" + particle_name + "_" + beforeafter_name,
            particle_title + " Z Vertex ; Z Vertex [cm]",
            200, -40, 40));

        // std::cout<<"Adding plots for "<<pdg<<" "<<beforeafter_name<<std::endl;
      }
      u_zvertexplots.insert({pdg, zvertexplots});
    }
  }


  bool ZVertexFilterValidator::RunHook(hipo::banklist& banks) const
  {
    auto& particle_bank = GetBank(banks, b_particle, "REC::Particle");

    // lock the mutex, so we can mutate plots
    std::scoped_lock<std::mutex> lock(m_mutex);

    // fill the plots before
    for(auto const& row : particle_bank.getRowList()) {
      double vz  = particle_bank.getFloat("vz", row);
      int pdg    = particle_bank.getInt("pid", row);
      int status = particle_bank.getShort("status", row);
      auto it    = u_zvertexplots.find(pdg);
      // check if pdg is amongs those that we want to plot
      if(it != u_zvertexplots.end() && abs(status) >= 2000) {
        u_zvertexplots.at(pdg).at(0)->Fill(vz);
      }
    }

    // run the momentum corrections
    m_algo_seq->Run(banks);

    // fill the plots after
    for(auto const& row : particle_bank.getRowList()) {
      double vz  = particle_bank.getFloat("vz", row);
      int pdg    = particle_bank.getInt("pid", row);
      int status = particle_bank.getShort("status", row);
      auto it    = u_zvertexplots.find(pdg);
      // check if pdg is amongs those that we want to plot
      if(it != u_zvertexplots.end() && abs(status) >= 2000) {
        u_zvertexplots.at(pdg).at(1)->Fill(vz);
      }
    }
    return true;
  }

  void ZVertexFilterValidator::StopHook()
  {
    if(GetOutputDirectory()) {
      for(auto const& [pdg, plots] : u_zvertexplots) {
        TString canv_name = Form("canv%d", pdg);
        auto canv         = new TCanvas(canv_name, canv_name, 800, 600);

        plots.at(0)->SetLineColor(kBlue);
        plots.at(0)->SetLineWidth(2);
        plots.at(0)->Draw("");

        plots.at(1)->SetLineColor(kRed);
        plots.at(1)->SetLineWidth(2);
        plots.at(1)->Draw("same");

        canv->Draw();
        canv->SaveAs(m_output_file_basename + "_" + std::to_string(pdg) + ".png");
      }
      m_output_file->Write();
      m_log->Info("Wrote output file {}", m_output_file->GetName());
      m_output_file->Close();
    }
  }

}
