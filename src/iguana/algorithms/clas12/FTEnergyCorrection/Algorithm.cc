#include "Algorithm.h"

namespace iguana::clas12 {
  REGISTER_IGUANA_ALGORITHM(FTEnergyCorrection);

  void FTEnergyCorrection::Start(hipo::banklist& banks) {
    b_ft_particle = GetBankIndex(banks, "RECFT::Particle");
    electron_mass = particle::mass.at(particle::electron);
  }

  void FTEnergyCorrection::Run(hipo::banklist& banks) const {
    auto& ftParticleBank = GetBank(banks, b_ft_particle, "RECFT::Particle");
    ShowBank(ftParticleBank, Logger::Header("INPUT FT PARTICLES"));
    for(auto const& row : ftParticleBank.getRowList()) {
      if(ftParticleBank.getInt("pid", row) == particle::PDG::electron) {
        auto px = ftParticleBank.getFloat("px", row);
        auto py = ftParticleBank.getFloat("py", row);
        auto pz = ftParticleBank.getFloat("pz", row);
        auto E = std::hypot(std::hypot(px, py, pz), electron_mass);
        auto [px_new, py_new, pz_new, E_new] = Transform(px, py, pz, E);
        ftParticleBank.putFloat("px", row, px_new);
        ftParticleBank.putFloat("py", row, py_new);
        ftParticleBank.putFloat("pz", row, pz_new);
      }
    }
    ShowBank(ftParticleBank, Logger::Header("OUTPUT FT PARTICLES"));
  }

  vector4_t  FTEnergyCorrection::Transform(
      vector_element_t px,
      vector_element_t py,
      vector_element_t pz,
      vector_element_t E) const
  {
    vector_element_t rho = std::hypot(px, py, pz);
    vector_element_t E_new = E +  0.0208922 + 0.050158*E - 0.0181107*pow(E,2) + 0.00305671*pow(E,3) - 0.000178235*pow(E,4);
    return { E_new*(px/rho), E_new*(py/rho), E_new*(pz/rho), E_new };
  }

  void FTEnergyCorrection::Stop() {
  }

}
