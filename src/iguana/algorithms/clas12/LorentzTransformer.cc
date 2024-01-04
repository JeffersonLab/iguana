#include "LorentzTransformer.h"

namespace iguana::clas12 {

  bool LorentzTransformer::s_registered = AlgorithmFactory::Register(
      LorentzTransformer::ClassName(),
      LorentzTransformer::Creator
      );


  void LorentzTransformer::Start(hipo::banklist& banks) {

    CacheOption("frame", std::string{"mirror"}, o_frame);
    CacheBankIndex(banks, "REC::Particle", b_particle);

    // define transformation function
    // TODO: add more useful frames, e.g., Breit
    if(o_frame == "mirror") { // reverses the momentum, just for demonstration
      m_transform = [] (lorentz_vector_t p) {
        auto [px, py, pz, e] = p;
        return lorentz_vector_t{-px, -py, -pz, -e};
      };
    }
    else {
      m_log->Error("unknown frame '{}'", o_frame);
      throw std::runtime_error("cannot Start LorentzTransformer algorithm");
    }

  }


  void LorentzTransformer::Run(hipo::banklist& banks) const {
    auto& particleBank = GetBank(banks, b_particle, "REC::Particle");
    ShowBank(particleBank, Logger::Header("INPUT PARTICLES"));
    for(int row = 0; row < particleBank.getRows(); row++) {
      auto [px, py, pz, e] = Transform(
          particleBank.getFloat("px", row),
          particleBank.getFloat("py", row),
          particleBank.getFloat("pz", row),
          0.0 // TODO: get the energy
          );
      particleBank.putFloat("px", row, px);
      particleBank.putFloat("py", row, py);
      particleBank.putFloat("pz", row, pz);
    }
    ShowBank(particleBank, Logger::Header("OUTPUT PARTICLES"));
  }


  lorentz_vector_t LorentzTransformer::Transform(lorentz_element_t px, lorentz_element_t py, lorentz_element_t pz, lorentz_element_t E) const {
    return m_transform({px, py, pz, E});
  }


  void LorentzTransformer::Stop() {
  }

}
