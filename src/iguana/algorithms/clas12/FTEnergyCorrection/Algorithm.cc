#include "Algorithm.h"

namespace iguana::clas12 {
  REGISTER_IGUANA_ALGORITHM(FTEnergyCorrection);

  void FTEnergyCorrection::Start(hipo::banklist& banks) {
    b_particle = GetBankIndex(banks, "REC::Particle");
    electron_mass = particle::mass.at(particle::electron);
  }

  void FTEnergyCorrection::Run(hipo::banklist& banks) const {
    auto& ParticleBank = GetBank(banks, b_particle, "REC::Particle");
    ShowBank(ParticleBank, Logger::Header("INPUT PARTICLES"));
    for(auto const& row : ParticleBank.getRowList()) {
      if(ParticleBank.getInt("pid", row) == particle::PDG::electron) {
        auto px = ParticleBank.getFloat("px", row);
        auto py = ParticleBank.getFloat("py", row);
        auto pz = ParticleBank.getFloat("pz", row);
        auto E = std::hypot(std::hypot(px, py, pz), electron_mass);
        auto [px_new, py_new, pz_new, E_new] = Transform(px, py, pz, E);
        ParticleBank.putFloat("px", row, px_new);
        ParticleBank.putFloat("py", row, py_new);
        ParticleBank.putFloat("pz", row, pz_new);
      }
    }
    ShowBank(ParticleBank, Logger::Header("OUTPUT PARTICLES"));
  }

  vector4_t  FTEnergyCorrection::Transform(
      vector_element_t px,
      vector_element_t py,
      vector_element_t pz,
      vector_element_t E) const
  {
    vector_element_t rho = std::hypot(px, py, pz);
    vector_element_t E_new = CorrectEnergy(E);
    return { E_new*(px/rho), E_new*(py/rho), E_new*(pz/rho), E_new };
  }

  vector_element_t FTEnergyCorrection::CorrectEnergy(vector_element_t E) const
  {
    return E +  0.0208922 + 0.050158*E - 0.0181107*pow(E,2) + 0.00305671*pow(E,3) - 0.000178235*pow(E,4);
  }

  void FTEnergyCorrection::Stop() {
  }

}
