#include "LorentzTransformer.h"

namespace iguana::clas12 {

  void LorentzTransformer::Start(hipo::banklist& banks) {

    CacheOption("frame", std::string{"mirror"}, o_frame);
    CacheBankIndex(banks, "REC::Particle", b_particle);

    // define transformation function
    // TODO: add more useful frames, e.g., Breit
    if(o_frame == "mirror") { // reverses the momentum, just for demonstration
      m_transform = [] (float& px, float& py, float& pz, float& e) {
        px = -px;
        py = -py;
        pz = -pz;
      };
    }
    else
      Throw(fmt::format("unknown frame '{}'", o_frame));

  }


  void LorentzTransformer::Run(hipo::banklist& banks) const {
    auto& particleBank = GetBank(banks, b_particle, "REC::Particle");
    ShowBank(particleBank, Logger::Header("INPUT PARTICLES"));
    for(int row = 0; row < particleBank.getRows(); row++) {
      auto px = particleBank.getFloat("px", row);
      auto py = particleBank.getFloat("py", row);
      auto pz = particleBank.getFloat("pz", row);
      float e = 0.0; // TODO: get the energy
      Transform(px, py, pz, e);
      particleBank.putFloat("px", row, px);
      particleBank.putFloat("py", row, py);
      particleBank.putFloat("pz", row, pz);
    }
    ShowBank(particleBank, Logger::Header("OUTPUT PARTICLES"));
  }


  void LorentzTransformer::Transform(float& px, float& py, float& pz, float& e) const {
    m_transform(px, py, pz, e);
  }


  void LorentzTransformer::Stop() {
  }

}
