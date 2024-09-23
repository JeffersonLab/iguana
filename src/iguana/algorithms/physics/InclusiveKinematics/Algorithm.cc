#include "Algorithm.h"

// ROOT
#include <Math/Vector4D.h>

namespace iguana::physics {

  REGISTER_IGUANA_ALGORITHM(InclusiveKinematics, "physics::InclusiveKinematics");

  void InclusiveKinematics::Start(hipo::banklist& banks)
  {
    b_particle = GetBankIndex(banks, "REC::Particle");
    b_config   = GetBankIndex(banks, "RUN::config");

    // create the output bank
    // FIXME: generalize the groupid and itemid
    auto result_schema = CreateBank(
        banks,
        b_result,
        GetClassName(),
        {"pindex/S", "Q2/D", "x/D", "y/D", "W/D", "nu/D", "qx/D", "qy/D", "qz/D", "qE/D"},
        0xF000,
        1);
    i_pindex = result_schema.getEntryOrder("pindex");
    i_Q2     = result_schema.getEntryOrder("Q2");
    i_x      = result_schema.getEntryOrder("x");
    i_y      = result_schema.getEntryOrder("y");
    i_W      = result_schema.getEntryOrder("W");
    i_nu     = result_schema.getEntryOrder("nu");
    i_qx     = result_schema.getEntryOrder("qx");
    i_qy     = result_schema.getEntryOrder("qy");
    i_qz     = result_schema.getEntryOrder("qz");
    i_qE     = result_schema.getEntryOrder("qE");

    // parse config file
    ParseYAMLConfig();
    o_runnum          = ConcurrentParamFactory::Create<int>();
    o_beam_energy     = ConcurrentParamFactory::Create<double>();
    o_beam_direction  = ConcurrentParamFactory::Create<std::vector<double>>();
    o_beam_particle   = ConcurrentParamFactory::Create<std::string>();
    o_target_particle = ConcurrentParamFactory::Create<std::string>();
    PrepareEvent(0); // load default values (using runnum==0)

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
    m_log->Debug("{:>30}: {}", "reconstruction method", method_reconstruction_str);
    m_log->Debug("{:>30}: {}", "lepton finder method", method_lepton_finder_str);
  }

  ///////////////////////////////////////////////////////////////////////////////

  void InclusiveKinematics::Run(hipo::banklist& banks) const
  {
    auto& particle_bank = GetBank(banks, b_particle, "REC::Particle");
    auto& config_bank   = GetBank(banks, b_config, "RUN::config");
    auto& result_bank   = GetBank(banks, b_result, GetClassName());
    ShowBank(particle_bank, Logger::Header("INPUT PARTICLES"));

    auto key = PrepareEvent(config_bank.getInt("run",0));

    auto lepton_pindex = FindScatteredLepton(particle_bank, key);
    if(lepton_pindex < 0) {
      ShowBank(result_bank, Logger::Header("CREATED BANK IS EMPTY"));
      return;
    }

    auto result_vars = ComputeFromLepton(
        particle_bank.getFloat("px", lepton_pindex),
        particle_bank.getFloat("py", lepton_pindex),
        particle_bank.getFloat("pz", lepton_pindex),
        key);

    result_bank.setRows(1);
    result_bank.putShort(i_pindex, 0, static_cast<int16_t>(lepton_pindex));
    result_bank.putDouble(i_Q2, 0, result_vars.Q2);
    result_bank.putDouble(i_x, 0, result_vars.x);
    result_bank.putDouble(i_y, 0, result_vars.y);
    result_bank.putDouble(i_W, 0, result_vars.W);
    result_bank.putDouble(i_nu, 0, result_vars.nu);
    result_bank.putDouble(i_qx, 0, result_vars.qx);
    result_bank.putDouble(i_qy, 0, result_vars.qy);
    result_bank.putDouble(i_qz, 0, result_vars.qz);
    result_bank.putDouble(i_qE, 0, result_vars.qE);

    ShowBank(result_bank, Logger::Header("CREATED BANK"));
  }

  ///////////////////////////////////////////////////////////////////////////////

  int InclusiveKinematics::FindScatteredLepton(hipo::bank const& particle_bank, concurrent_key_t const key) const
  {
    int lepton_row       = -1;
    double lepton_energy = 0;

    switch(o_method_lepton_finder) {
    case method_lepton_finder::highest_energy_FD_trigger: {
      // find highest energy lepton
      auto beam = m_beam->Load(key);
      for(auto const& row : particle_bank.getRowList()) {
        if(particle_bank.getInt("pid", row) == beam.pdg) {
          double px = particle_bank.getFloat("px", row);
          double py = particle_bank.getFloat("py", row);
          double pz = particle_bank.getFloat("pz", row);
          double en = std::sqrt(std::pow(px, 2) + std::pow(py, 2) + std::pow(pz, 2) + std::pow(beam.mass, 2));
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
    if(lepton_row >= 0)
      m_log->Debug("Found scattered lepton: row={}, energy={}", lepton_row, lepton_energy);
    else
      m_log->Debug("Scattered lepton not found");
    return lepton_row;
  }

  ///////////////////////////////////////////////////////////////////////////////

  concurrent_key_t InclusiveKinematics::PrepareEvent(int const runnum, concurrent_key_t key) const
  {
    m_log->Trace("calling PrepareEvent({}, {})", runnum, key);
    if(o_runnum->NeedsHashing()) {
      std::hash<int> hash_ftn;
      auto hash_key = hash_ftn(runnum);
      if(!o_runnum->HasKey(hash_key))
        Reload(runnum, hash_key);
      return hash_key;
    }
    if(o_runnum->IsEmpty() || o_runnum->Load(key) != runnum)
      Reload(runnum, key);
    return key;
  }

  ///////////////////////////////////////////////////////////////////////////////

  void InclusiveKinematics::Reload(int const runnum, concurrent_key_t key) const
  {
    std::lock_guard<std::mutex> const lock(m_mutex);
    m_log->Trace("-> calling Reload({}, {})", runnum, key);
    o_runnum->Save(runnum, key);

    auto beam_energy     = GetOptionScalar<double>("beam_energy", {"initial_state", GetConfig()->InRange("runs", runnum), "beam_energy"});
    auto beam_direction  = GetOptionVector<double>("beam_direction", {"initial_state", GetConfig()->InRange("runs", runnum), "beam_direction"});
    auto beam_particle   = GetOptionScalar<std::string>("beam_particle", {"initial_state", GetConfig()->InRange("runs", runnum), "beam_particle"});
    auto target_particle = GetOptionScalar<std::string>("target_particle", {"initial_state", GetConfig()->InRange("runs", runnum), "target_particle"});

    particle_t beam, target;

    beam.pdg        = 0;
    beam.mass       = -1.0;
    target.mass     = -1.0;
    for(auto const& [pdg, name] : particle::name) {
      if(name == beam_particle) {
        beam.pdg  = pdg;
        beam.mass = particle::mass.at(pdg);
      }
      if(name == target_particle)
        target.mass = particle::mass.at(pdg);
    }
    if(beam.pdg == 0) {
      m_log->Error("Unknown beam particle {:?}", beam_particle);
      throw std::runtime_error("Start failed");
    }
    if(target.mass < 0.0) {
      m_log->Error("Unknown target particle {:?}", target_particle);
      throw std::runtime_error("Start failed");
    }

    // set the beam and target momenta
    if(beam_direction.size() != 3) {
      m_log->Error("Beam direction is not a 3-vector; assuming it is (0, 0, 1) instead");
      beam_direction = {0.0, 0.0, 1.0};
    }
    auto dir_mag = std::hypot(beam_direction[0], beam_direction[1], beam_direction[2]);
    auto beam_p  = std::sqrt(std::pow(beam_energy, 2) - std::pow(beam.mass, 2));
    if(dir_mag > 0) {
      beam.px = beam_direction[0] * beam_p / dir_mag;
      beam.py = beam_direction[1] * beam_p / dir_mag;
      beam.pz = beam_direction[2] * beam_p / dir_mag;
    }
    else {
      m_log->Error("Beam direction magnitude is not > 0");
      throw ::std::runtime_error("Start failed");
    }
    target.px = 0.0;
    target.py = 0.0;
    target.pz = 0.0;

    o_beam_energy->Save(beam_energy, key);
    o_beam_direction->Save(beam_direction, key);
    o_beam_particle->Save(beam_particle, key);
    o_target_particle->Save(target_particle, key);
    m_beam->Save(beam, key);
    m_target->Save(target, key);

    m_log->Debug("{:>30}: {}", "beam energy", beam_energy);
    m_log->Debug("{:>30}: {}, mass = {}, p = ({}, {}, {})", "beam particle", beam_particle, beam.mass, beam.px, beam.py, beam.pz);
    m_log->Debug("{:>30}: {}, mass = {}, p = ({}, {}, {})", "target particle", target_particle, target.mass, target.px, target.py, target.pz);
  }

  ///////////////////////////////////////////////////////////////////////////////

  InclusiveKinematicsVars InclusiveKinematics::ComputeFromLepton(
      vector_element_t const lepton_px,
      vector_element_t const lepton_py,
      vector_element_t const lepton_pz,
      concurrent_key_t const key
      ) const
  {
    InclusiveKinematicsVars result;

    m_log->Trace("Reconstruct inclusive kinematics from lepton with p=({}, {}, {})", lepton_px, lepton_py, lepton_pz);

    auto beam   = m_beam->Load(key);
    auto target = m_target->Load(key);

    ROOT::Math::PxPyPzMVector vec_beam(beam.px, beam.py, beam.pz, beam.mass);
    ROOT::Math::PxPyPzMVector vec_target(target.px, target.py, target.pz, target.mass);
    ROOT::Math::PxPyPzMVector vec_lepton(lepton_px, lepton_py, lepton_pz, beam.mass);

    auto vec_q = vec_beam - vec_lepton;
    result.qx  = vec_q.Px();
    result.qy  = vec_q.Py();
    result.qz  = vec_q.Pz();
    result.qE  = vec_q.E();
    result.Q2  = -1 * vec_q.M2();
    result.x   = result.Q2 / (2 * vec_q.Dot(vec_target));
    result.y   = vec_target.Dot(vec_q) / vec_target.Dot(vec_beam);
    result.W   = (vec_beam + vec_target - vec_lepton).M();
    result.nu  = vec_target.Dot(vec_q) / target.mass;

    return result;
  }

  ///////////////////////////////////////////////////////////////////////////////

  void InclusiveKinematics::Stop()
  {
  }

}
