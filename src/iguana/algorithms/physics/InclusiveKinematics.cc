#include "InclusiveKinematics.h"

// ROOT
#include <Math/Vector4D.h>

namespace iguana::physics {

  REGISTER_IGUANA_ALGORITHM(InclusiveKinematics);

  void InclusiveKinematics::Start(hipo::banklist& banks)
  {
    b_particle = GetBankIndex(banks, "REC::Particle");

    ParseYAMLConfig();

    o_runnum = GetCachedOption<int>("runnum").value_or(0); // FIXME: should be set from RUN::conig

    // get initial state configuration
    o_beam_energy     = GetOptionScalar<double>("beam_energy", {"initial_state", GetConfig()->InRange("runs", o_runnum), "beam_energy"});
    o_beam_direction  = GetOptionVector<double>("beam_direction", {"initial_state", GetConfig()->InRange("runs", o_runnum), "beam_direction"});
    o_beam_particle   = GetOptionScalar<std::string>("beam_particle", {"initial_state", GetConfig()->InRange("runs", o_runnum), "beam_particle"});
    o_target_particle = GetOptionScalar<std::string>("target_particle", {"initial_state", GetConfig()->InRange("runs", o_runnum), "target_particle"});
    m_beam.pdg        = 0;
    m_beam.mass       = -1.0;
    m_target.mass     = -1.0;
    for(const auto& [pdg, name] : particle::name) {
      if(name == o_beam_particle) {
        m_beam.pdg  = pdg;
        m_beam.mass = particle::mass.at(pdg);
      }
      if(name == o_target_particle)
        m_target.mass = particle::mass.at(pdg);
    }
    if(m_beam.pdg == 0) {
      m_log->Error("Unknown beam particle {:?}", o_beam_particle);
      throw std::runtime_error("Start failed");
    }
    if(m_target.mass < 0.0) {
      m_log->Error("Unknown target particle {:?}", o_target_particle);
      throw std::runtime_error("Start failed");
    }

    // get reconstruction method configuration
    auto method_reconstruction_str = GetOptionScalar<std::string>("method_reconstruction", {"method", "reconstruction"});
    if(method_reconstruction_str == "scattered_lepton") {
      o_method_reconstruction = method_reconstruction::scattered_lepton;
    }
    else {
      m_log->Error("Unknown reconstruction method {:?}", method_reconstruction_str);
      throw std::runtime_error("Start failed");
    }

    // get scattered lepton finder configuration
    auto method_lepton_finder_str = GetOptionScalar<std::string>("method_lepton_finder", {"method", "lepton_finder"});
    if(method_lepton_finder_str == "highest_energy_FD_trigger") {
      o_method_lepton_finder = method_lepton_finder::highest_energy_FD_trigger;
    }
    else {
      m_log->Error("Unknown lepton finder method {:?}", method_lepton_finder_str);
      throw std::runtime_error("Start failed");
    }

    // set the beam and target momenta
    if(o_beam_direction.size() != 3) {
      m_log->Error("Beam direction is not a 3-vector; assuming it is (0, 0, 1) instead");
      o_beam_direction = {0.0, 0.0, 1.0};
    }
    auto dir_mag = std::hypot(o_beam_direction[0], o_beam_direction[1], o_beam_direction[2]);
    auto beam_p  = std::sqrt(std::pow(o_beam_energy, 2) - std::pow(m_beam.mass, 2));
    if(dir_mag > 0) {
      m_beam.px = o_beam_direction[0] * beam_p / dir_mag;
      m_beam.py = o_beam_direction[1] * beam_p / dir_mag;
      m_beam.pz = o_beam_direction[2] * beam_p / dir_mag;
    }
    else {
      m_log->Error("Beam direction magnitude is not > 0");
      throw::std::runtime_error("Start failed");
    }
    m_target.px = 0.0;
    m_target.py = 0.0;
    m_target.pz = 0.0;

    // print the configuration
    m_log->Debug(Logger::Header("CONFIGURATION"));
    m_log->Debug("{:>30}: {}", "beam energy", o_beam_energy);
    m_log->Debug("{:>30}: {}, mass = {}, p = ({}, {}, {})", "beam particle", o_beam_particle, m_beam.mass, m_beam.px, m_beam.py, m_beam.pz);
    m_log->Debug("{:>30}: {}, mass = {}, p = ({}, {}, {})", "target particle", o_target_particle, m_target.mass, m_target.px, m_target.py, m_target.pz);
    m_log->Debug("{:>30}: {}", "reconstruction method", method_reconstruction_str);
    m_log->Debug("{:>30}: {}", "lepton finder method", method_lepton_finder_str);
  }

  ///////////////////////////////////////////////////////////////////////////////

  void InclusiveKinematics::Run(hipo::banklist& banks) const
  {
    auto& particle_bank = GetBank(banks, b_particle, "REC::Particle");
    ShowBank(particle_bank, Logger::Header("INPUT PARTICLES"));

    auto lepton_pindex = FindScatteredLepton(particle_bank);
    auto result_vars = ComputeFromLepton(
        particle_bank.getFloat("px", lepton_pindex),
        particle_bank.getFloat("py", lepton_pindex),
        particle_bank.getFloat("pz", lepton_pindex),
        m_beam.px,
        m_beam.py,
        m_beam.pz,
        m_target.mass,
        m_beam.pdg
        );

    // TODO: create a new bank with these variables
  }

  ///////////////////////////////////////////////////////////////////////////////

  int InclusiveKinematics::FindScatteredLepton(const hipo::bank& particle_bank) const
  {
    int lepton_row = -1;

    switch(o_method_lepton_finder) {
      case method_lepton_finder::highest_energy_FD_trigger:
        {
          // find highest energy lepton
          double lepton_energy = 0;
          for(int row = 0; row < particle_bank.getRows(); row++) {
            if(particle_bank.getInt("pid", row) == m_beam.pdg) {
              double px = particle_bank.getInt("px", row);
              double py = particle_bank.getInt("py", row);
              double pz = particle_bank.getInt("pz", row);
              double en = std::sqrt(std::pow(px, 2) + std::pow(py, 2) + std::pow(pz, 2) + std::pow(m_beam.mass, 2));
              if(en > lepton_energy) {
                lepton_row    = row;
                lepton_energy = en;
              }
            }
          }
          // require it to be in the FD trigger
          if(lepton_row >= 0) {
            auto status = particle_bank.getShort("status", lepton_row);
            if(status <= -3000 || status > -2000)
              lepton_row = -1;
          }
          break;
        }
    }
    return lepton_row;
  }

  ///////////////////////////////////////////////////////////////////////////////

  InclusiveKinematicsVars InclusiveKinematics::ComputeFromLepton(
      vector_element_t lepton_px,
      vector_element_t lepton_py,
      vector_element_t lepton_pz,
      vector_element_t beam_px,
      vector_element_t beam_py,
      vector_element_t beam_pz,
      double target_mass,
      double lepton_pdg
      ) const
  {
    if(target_mass <= 0) {
      m_log->Error("target mass is not > 0");
      throw std::runtime_error("Run failed");
    }

    InclusiveKinematicsVars result;

    ROOT::Math::PxPyPzMVector vec_beam(m_beam.px, m_beam.px, m_beam.px, m_beam.mass);
    ROOT::Math::PxPyPzMVector vec_target(m_target.px, m_target.py, m_target.pz, m_target.mass);
    ROOT::Math::PxPyPzMVector vec_lepton(lepton_px, lepton_py, lepton_pz, m_beam.mass);

    auto vec_q = vec_beam - vec_lepton;
    result.q   = {vec_q.Px(), vec_q.Py(), vec_q.Pz(), vec_q.E()};
    result.Q2  = -1 * vec_q.M2();
    result.xB  = result.Q2 / ( 2 * vec_q.Dot(vec_target) );
    result.y   = vec_target.Dot(vec_q) / vec_target.Dot(vec_beam);
    result.W   = (vec_beam + vec_target - vec_lepton).M();
    result.nu  = vec_target.Dot(vec_q) / target_mass;

    return result;
  }

  ///////////////////////////////////////////////////////////////////////////////

  void InclusiveKinematics::Stop()
  {
  }

}