#include "Algorithm.h"

// ROOT
#include <Math/Boost.h>
#include <Math/Vector4D.h>

namespace iguana::clas12 {

  REGISTER_IGUANA_ALGORITHM(LorentzTransformer);

  void LorentzTransformer::Start(hipo::banklist& banks)
  {

    // define options, their default values, and cache them
    ParseYAMLConfig();
    o_frame = GetOptionScalar<std::string>("frame");

    // define transformation function
    // TODO: add more useful frames, e.g., Breit, but they require other momenta, such as q;
    // for now we just have a boost to the beam electron frame, as an example, since that just
    // requires the beam energy
    if(o_frame == "beam_rest_frame") { // beam electron rest frame
      m_transformation_type = e_boost;
      o_beam_energy         = GetCachedOption<double>("beam_energy").value_or(10.6); // FIXME
    }
    else {
      m_log->Error("unknown frame '{}'", o_frame);
      throw std::runtime_error("cannot Start LorentzTransformer algorithm");
    }

    b_particle = GetBankIndex(banks, "REC::Particle");
  }


  void LorentzTransformer::Run(hipo::banklist& banks) const
  {

    auto& particleBank = GetBank(banks, b_particle, "REC::Particle");
    ShowBank(particleBank, Logger::Header("INPUT PARTICLES"));

    // boosts
    if(m_transformation_type == e_boost) {
      // set the boost vector
      std::tuple<vector_element_t, vector_element_t, vector_element_t> boost_vec;
      if(o_frame == "beam_rest_frame") {
        boost_vec = {0, 0, o_beam_energy / std::hypot(o_beam_energy, 0.000511)};
      }
      // boost each particle
      for(auto const& row : particleBank.getRowList()) {
        auto [px, py, pz, e] = Boost(
            particleBank.getFloat("px", row),
            particleBank.getFloat("py", row),
            particleBank.getFloat("pz", row),
            0.0, // FIXME: get the energy
            std::get<0>(boost_vec),
            std::get<1>(boost_vec),
            std::get<2>(boost_vec));
        particleBank.putFloat("px", row, px);
        particleBank.putFloat("py", row, py);
        particleBank.putFloat("pz", row, pz);
      }
    }
    ShowBank(particleBank, Logger::Header("OUTPUT PARTICLES"));
  }


  Momentum4 LorentzTransformer::Boost(
      vector_element_t const px,
      vector_element_t const py,
      vector_element_t const pz,
      vector_element_t const E,
      vector_element_t const beta_x,
      vector_element_t const beta_y,
      vector_element_t const beta_z) const
  {
    m_log->Debug(fmt::format("{::<30}", "Boost "));
    m_log->Debug(fmt::format("{:>8} = ({:10f}, {:10f}, {:10f}, {:10f})", "p_in", px, py, pz, E));

    // check if |beta| <= 1
    auto beta_mag = std::hypot(beta_x, beta_y, beta_z);
    if(beta_mag > 1) {
      m_log->Error("attempt to boost with beta > 1 (faster than the speed of light); will NOT boost this momentum");
      m_log->Debug("{:>8} = {}", "|beta|", beta_mag);
      m_log->Debug("{:>8} = ({:10f}, {:10f}, {:10f})", "beta", beta_x, beta_y, beta_z);
      return {px, py, pz, E};
    }

    // boost
    ROOT::Math::PxPyPzEVector p_in(px, py, pz, E);
    ROOT::Math::Boost beta(beta_x, beta_y, beta_z);
    auto p_out = beta(p_in);

    if(m_log->GetLevel() <= Logger::Level::debug) {
      m_log->Debug(fmt::format("{:>8} = ({:10f}, {:10f}, {:10f})", "beta", beta.BetaVector().X(), beta.BetaVector().Y(), beta.BetaVector().Z()));
      m_log->Debug(fmt::format("{:>8} = ({:10f}, {:10f}, {:10f}, {:10f})", "p_out", p_out.Px(), p_out.Py(), p_out.Pz(), p_out.E()));
    }
    return {p_out.Px(), p_out.Py(), p_out.Pz(), p_out.E()};
  }


  void LorentzTransformer::Stop()
  {
  }

}
