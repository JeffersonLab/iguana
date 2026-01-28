#include "Algorithm.h"

namespace iguana::clas12::rga {
  REGISTER_IGUANA_ALGORITHM(FTEnergyCorrection);

  void FTEnergyCorrection::StartHook(hipo::banklist& banks)
  {
    b_ft_particle = GetBankIndex(banks, "RECFT::Particle");
    electron_mass = particle::mass.at(particle::electron);
  }

  bool FTEnergyCorrection::RunHook(hipo::banklist& banks) const
  {
    return Run(GetBank(banks, b_ft_particle, "RECFT::Particle"));
  }

  bool FTEnergyCorrection::Run(hipo::bank& ftParticleBank) const
  {
    ShowBank(ftParticleBank, Logger::Header("INPUT FT PARTICLES"));
    for(auto const& row : ftParticleBank.getRowList()) {
      if(ftParticleBank.getInt("pid", row) == particle::PDG::electron) {
        auto px                              = ftParticleBank.getFloat("px", row);
        auto py                              = ftParticleBank.getFloat("py", row);
        auto pz                              = ftParticleBank.getFloat("pz", row);
        auto E                               = std::hypot(std::hypot(px, py, pz), electron_mass);
        auto [px_new, py_new, pz_new, E_new] = Transform(px, py, pz, E);
        ftParticleBank.putFloat("px", row, px_new);
        ftParticleBank.putFloat("py", row, py_new);
        ftParticleBank.putFloat("pz", row, pz_new);
      }
    }
    ShowBank(ftParticleBank, Logger::Header("OUTPUT FT PARTICLES"));
    return true;
  }

  Momentum4 FTEnergyCorrection::Transform(
      vector_element_t const px,
      vector_element_t const py,
      vector_element_t const pz,
      vector_element_t const E) const
  {
    vector_element_t rho   = std::hypot(px, py, pz);
    vector_element_t E_new = CorrectEnergy(E);
    return {E_new * (px / rho), E_new * (py / rho), E_new * (pz / rho), E_new};
  }

  vector_element_t FTEnergyCorrection::CorrectEnergy(vector_element_t const E) const
  {
    return E + 0.0208922 + 0.050158 * E - 0.0181107 * pow(E, 2) + 0.00305671 * pow(E, 3) - 0.000178235 * pow(E, 4);
  }

}
