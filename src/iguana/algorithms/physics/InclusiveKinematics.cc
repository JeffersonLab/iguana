#include "InclusiveKinematics.h"

namespace iguana::physics {

  REGISTER_IGUANA_ALGORITHM(InclusiveKinematics);

  void InclusiveKinematics::Start(hipo::banklist& banks)
  {
    b_particle = GetBankIndex(banks, "REC::Particle");

    ParseYAMLConfig();

    o_runnum = GetCachedOption<int>("runnum").value_or(0); // FIXME: should be set from RUN::conig

    // get initial state configuration
    o_beam_energy          = GetOptionScalar<double>("beam_energy", {"initial_state", GetConfig()->InRange("runs", o_runnum), "beam_energy"});
    o_beam_particle        = GetOptionScalar<std::string>("beam_particle", {"initial_state", GetConfig()->InRange("runs", o_runnum), "beam_particle"});
    o_target_particle      = GetOptionScalar<std::string>("target_particle", {"initial_state", GetConfig()->InRange("runs", o_runnum), "target_particle"});
    m_beam_particle_mass   = -1.0;
    m_target_particle_mass = -1.0;
    for(const auto& [pdg, name] : particle::name) {
      if(name == o_beam_particle)
        m_beam_particle_mass = particle::mass.at(pdg);
      if(name == o_target_particle)
        m_target_particle_mass = particle::mass.at(pdg);
    }
    if(m_beam_particle_mass < 0.0) {
      m_log->Error("Unknown beam particle {:?}", o_beam_particle);
      throw std::runtime_error("Start failed");
    }
    if(m_target_particle_mass < 0.0) {
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

    // print the configuration
    m_log->Debug(Logger::Header("CONFIGURATION"));
    m_log->Debug("{:>30}: {}", "beam energy", o_beam_energy);
    m_log->Debug("{:>30}: {}, mass = {}", "beam particle", o_beam_particle, m_beam_particle_mass);
    m_log->Debug("{:>30}: {}, mass = {}", "target particle", o_target_particle, m_target_particle_mass);
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
        o_beam_energy,
        m_target_particle_mass,
        0.0,
        0.0,
        1.0,
        m_beam_particle_mass
        );

    // TODO: create a new bank with these variables
  }

  ///////////////////////////////////////////////////////////////////////////////

  int InclusiveKinematics::FindScatteredLepton(const hipo::bank& particle_bank) const
  {
    // TODO
    return 0;
  }

  ///////////////////////////////////////////////////////////////////////////////

  InclusiveKinematicsVars InclusiveKinematics::ComputeFromLepton(
      vector_element_t lepton_px,
      vector_element_t lepton_py,
      vector_element_t lepton_pz,
      double beam_E,
      double target_M,
      vector_element_t beam_dir_x,
      vector_element_t beam_dir_y,
      vector_element_t beam_dir_z,
      double lepton_M
      ) const
  {
    // TODO
    return {};
  }

  ///////////////////////////////////////////////////////////////////////////////

  void InclusiveKinematics::Stop()
  {
  }

}
