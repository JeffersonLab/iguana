#include "FTEnergyCorrectionValidator.h"
#include <TProfile.h>
#include <TStyle.h>

//Edit  m_E_max, -m_deltaE_max and m_deltaE_max.
//See what the plotting does.
namespace iguana::clas12 {

  REGISTER_IGUANA_VALIDATOR(FTEnergyCorrectionValidator);

  void FTEnergyCorrectionValidator::Start(hipo::banklist& banks)
  {
    // define the algorithm sequence
    m_algo_seq = std::make_unique<AlgorithmSequence>();
    m_algo_seq->Add("clas12::FT_Energy_Corr");
    m_algo_seq->SetOption<std::vector<int>>("clas12::EventBuilderFilter", "pids", u_pdg_list);
    m_algo_seq->Start(banks);

    // get bank indices
    b_particle = GetBankIndex(banks, "REC::Particle");
    electron_mass = particle::mass.at(particle::electron);

    // set an output file
    auto output_dir = GetOutputDirectory();
    if(output_dir) {
      m_output_file_basename = output_dir.value() + "/energy_corrections";
      m_output_file          = new TFile(m_output_file_basename + ".root", "RECREATE");

    }
    InitializeHistograms();
    }


  void FTEnergyCorrectionValidator::Run(hipo::banklist& banks) const
  {
    // get the momenta before
    auto& ParticleBank = GetBank(banks, b_particle, "REC::Particle");
    double px_el, py_el, pz_el, E_el;
    double px_pim, py_pim, pz_pim;
    double px_pip, py_pip, pz_pip;
    double px_pr, py_pr, pz_pr;
    double DeltaE;
    TLorentzVector beam(0,0,10.6,10.6);
    TLorentzVector target(0,0,0,0.938272);
    TLorentzVector miss_el, pion_plus, pion_minus, proton, electron;
    

    for(auto const& row : particle_bank.getRowList()){

          if(ParticleBank.getInt("pid", row) == particle::PDG::electron){

            px_el = ParticleBank.getFloat("px", row);
            py_el = ParticleBank.getFloat("py", row);
            pz_el = ParticleBank.getFloat("pz", row);
            electron.SetXYZM(px_el, py_el, pz_el, electron_mass);
          }

          if(ParticleBank.getInt("pid", row) == particle::PDG::pion_minus){

            px_pim = ParticleBank.getFloat("px", row);
            py_pim = ParticleBank.getFloat("py", row);
            pz_pim = ParticleBank.getFloat("pz", row);
            pion_minus.SetXYZM(px_pim, py_pim, pz_pim, 0.139600);
          }

          if(ParticleBank.getInt("pid", row) == particle::PDG::pion_plus){

            px_pip = ParticleBank.getFloat("px", row);
            py_pip = ParticleBank.getFloat("py", row);
            pz_pip = ParticleBank.getFloat("pz", row);
            pion_plus.SetXYZM(px_pip, py_pip, pz_pip, 0.139600);
          }

          if(ParticleBank.getInt("pid", row) == particle::PDG::proton){

            px_pr = ParticleBank.getFloat("px", row);
            py_pr = ParticleBank.getFloat("py", row);
            pz_pr = ParticleBank.getFloat("pz", row);
            proton.SetXYZM(px_pr, py_pr, pz_pr, 0.938272);
          }
          miss_el =  beam + target - pion_minus - pion_plus - proton;
          DeltaE = miss_el.E() - el.E();
          h_beforecorr->Fill(el.E(), DeltaE);
    }
    // run the energy corrections
    m_algo_seq->Run(banks);

    for(auto const& row : particle_bank.getRowList()){

          if(ParticleBank.getInt("pid", row) == particle::PDG::electron){

            px_el = ParticleBank.getFloat("px", row);
            py_el = ParticleBank.getFloat("py", row);
            pz_el = ParticleBank.getFloat("pz", row);
            electron.SetXYZM(px_el, py_el, pz_el, electron_mass);
          }

          if(ParticleBank.getInt("pid", row) == particle::PDG::pion_minus){

            px_pim = ParticleBank.getFloat("px", row);
            py_pim = ParticleBank.getFloat("py", row);
            pz_pim = ParticleBank.getFloat("pz", row);
            pion_minus.SetXYZM(px_pim, py_pim, pz_pim, 0.139600);
          }

          if(ParticleBank.getInt("pid", row) == particle::PDG::pion_plus){

            px_pip = ParticleBank.getFloat("px", row);
            py_pip = ParticleBank.getFloat("py", row);
            pz_pip = ParticleBank.getFloat("pz", row);
            pion_plus.SetXYZM(px_pip, py_pip, pz_pip, 0.139600);
          }

          if(ParticleBank.getInt("pid", row) == particle::PDG::proton){

            px_pr = ParticleBank.getFloat("px", row);
            py_pr = ParticleBank.getFloat("py", row);
            pz_pr = ParticleBank.getFloat("pz", row);
            proton.SetXYZM(px_pr, py_pr, pz_pr, 0.938272);
          }
          miss_el =  beam + target - pion_minus - pion_plus - proton;
          DeltaE = miss_el.E() - el.E();
          h_aftercorr->Fill(el.E(), DeltaE);
    }
      
  }
  void FTEnergyCorrectionValidator::Stop()
  {
    if(GetOutputDirectory()) {

          int n_rows = 1;
          int n_cols = 2;
          auto canv = new TCanvas("c","c",n_cols*800,n_rows*800);
          canv->Divide(n_cols,n_rows);
          gStyle->SetOptStat(0);
          for(int pad_num = 0; pad_num < 2; pad_num++){
              auto pad = canv->GetPad(pad_num+1);
              pad->cd();
              pad->SetGrid(1, 1);
              pad->SetLogz();
              pad->SetLeftMargin(0.12);
              pad->SetRightMargin(0.12);
              pad->SetBottomMargin(0.12);

              switch(pad_num){
                  case 0:
                      h_beforecorr->Draw("colz");
                      break;
                  case 1:
                      h_aftercorr->Draw("colz")
                      break;
              }
          }
      canv->SaveAs(m_output_file_basename + "_plot.png");
      m_output_file->Write();
      m_log->Info("Wrote output file {}", m_output_file->GetName());
      m_output_file->Close();
    }
  }

}
