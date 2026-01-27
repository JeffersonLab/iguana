#include "Algorithm.h"

// ROOT
#include <Math/Vector3D.h>
#include <Math/Vector4D.h>
#include <TMath.h>

namespace iguana::physics {

  REGISTER_IGUANA_ALGORITHM(InclusiveKinematics, "physics::InclusiveKinematics");

  ///////////////////////////////////////////////////////////////////////////////

  void InclusiveKinematics::ConfigHook()
  {
    // parse config file
    o_particle_bank           = GetOptionScalar<std::string>({"particle_bank"});
    o_runnum                  = ConcurrentParamFactory::Create<int>();
    o_target_PxPyPzM          = ConcurrentParamFactory::Create<std::vector<double>>();
    o_beam_PxPyPzM            = ConcurrentParamFactory::Create<std::vector<double>>();
    o_theta_between_FD_and_FT = GetOptionScalar<double>({"theta_between_FD_and_FT"});

    // get reconstruction method configuration
    auto method_reconstruction_str = GetOptionScalar<std::string>({"method", "reconstruction"});
    if(method_reconstruction_str == "scattered_lepton") {
      o_method_reconstruction = method_reconstruction::scattered_lepton;
    }
    else {
      m_log->Error("Unknown reconstruction method {:?}", method_reconstruction_str);
      throw std::runtime_error("Start failed");
    }

    // get scattered lepton finder configuration
    auto method_lepton_finder_str = GetOptionScalar<std::string>({"method", "lepton_finder"});
    if(method_lepton_finder_str == "highest_energy_FD_trigger")
      o_method_lepton_finder = method_lepton_finder::highest_energy_FD_trigger;
    else if(method_lepton_finder_str == "lund_beam_daughter")
      o_method_lepton_finder = method_lepton_finder::lund_beam_daughter;
    else {
      m_log->Error("Unknown lepton finder method {:?}", method_lepton_finder_str);
      throw std::runtime_error("Start failed");
    }

    // get beam PDG and mass
    o_beam_pdg         = 0;
    auto beam_particle = GetOptionScalar<std::string>({"method", "beam_particle"});
    for(auto const& [pdg, name] : particle::name) {
      if(name == beam_particle) {
        o_beam_pdg  = pdg;
        o_beam_mass = particle::mass.at(pdg);
        break;
      }
    }
    if(o_beam_pdg == 0) {
      m_log->Error("Unknown beam particle {:?}", beam_particle);
      throw std::runtime_error("Start failed");
    }

  }

  ///////////////////////////////////////////////////////////////////////////////

  void InclusiveKinematics::StartHook(hipo::banklist& banks)
  {
    // get bank indices
    b_particle = GetBankIndex(banks, o_particle_bank);
    b_config   = GetBankIndex(banks, "RUN::config");

    // create the output bank
    auto result_schema = CreateBank(banks, b_result, GetClassName());
    i_pindex           = result_schema.getEntryOrder("pindex");
    i_Q2               = result_schema.getEntryOrder("Q2");
    i_x                = result_schema.getEntryOrder("x");
    i_y                = result_schema.getEntryOrder("y");
    i_W                = result_schema.getEntryOrder("W");
    i_nu               = result_schema.getEntryOrder("nu");
    i_qx               = result_schema.getEntryOrder("qx");
    i_qy               = result_schema.getEntryOrder("qy");
    i_qz               = result_schema.getEntryOrder("qz");
    i_qE               = result_schema.getEntryOrder("qE");
    i_beamPz           = result_schema.getEntryOrder("beamPz");
    i_targetM          = result_schema.getEntryOrder("targetM");

    // instantiate RCDB reader `m_rcdb`
    StartRCDBReader();
    o_override_beam_energy = GetOptionScalar<double>({"override_beam_energy"}); // FIXME: should go in `ConfigHook`?
    if(o_override_beam_energy > 0)
      m_rcdb->SetBeamEnergyOverride(o_override_beam_energy);
  }

  ///////////////////////////////////////////////////////////////////////////////

  bool InclusiveKinematics::RunHook(hipo::banklist& banks) const
  {
    return Run(
        GetBank(banks, b_particle, o_particle_bank),
        GetBank(banks, b_config, "RUN::config"),
        GetBank(banks, b_result, GetClassName()));
  }

  bool InclusiveKinematics::Run(
      hipo::bank const& particle_bank,
      hipo::bank const& config_bank,
      hipo::bank& result_bank) const
  {
    result_bank.reset(); // IMPORTANT: always first `reset` the created bank(s)
    ShowBank(particle_bank, Logger::Header("INPUT PARTICLES"));

    auto key = PrepareEvent(config_bank.getInt("run", 0));

    auto const lepton_pindex = FindScatteredLepton(particle_bank, key);
    if(!lepton_pindex.has_value()) {
      ShowBank(result_bank, Logger::Header("CREATED BANK IS EMPTY"));
      return false;
    }

    auto result_vars = ComputeFromLepton(
        particle_bank.getFloat("px", lepton_pindex.value()),
        particle_bank.getFloat("py", lepton_pindex.value()),
        particle_bank.getFloat("pz", lepton_pindex.value()),
        key);
    result_vars.pindex = lepton_pindex.value(); // FIXME: should be done in `ComputeFromLepton`, but need a proper action function first...

    result_bank.setRows(1);
    result_bank.putShort(i_pindex, 0, static_cast<int16_t>(result_vars.pindex));
    result_bank.putDouble(i_Q2, 0, result_vars.Q2);
    result_bank.putDouble(i_x, 0, result_vars.x);
    result_bank.putDouble(i_y, 0, result_vars.y);
    result_bank.putDouble(i_W, 0, result_vars.W);
    result_bank.putDouble(i_nu, 0, result_vars.nu);
    result_bank.putDouble(i_qx, 0, result_vars.qx);
    result_bank.putDouble(i_qy, 0, result_vars.qy);
    result_bank.putDouble(i_qz, 0, result_vars.qz);
    result_bank.putDouble(i_qE, 0, result_vars.qE);
    result_bank.putDouble(i_beamPz, 0, result_vars.beamPz);
    result_bank.putDouble(i_targetM, 0, result_vars.targetM);

    ShowBank(result_bank, Logger::Header("CREATED BANK"));
    return true;
  }

  ///////////////////////////////////////////////////////////////////////////////

  std::optional<int> const InclusiveKinematics::FindScatteredLepton(hipo::bank const& particle_bank, concurrent_key_t const key) const
  {
    std::optional<int> lepton_row = std::nullopt;
    double lepton_energy          = 0.0;

    switch(o_method_lepton_finder) {
    // ----------------------------------------------------------------------------------
    // highest energy FD trigger lepton
    // ----------------------------------------------------------------------------------
    case method_lepton_finder::highest_energy_FD_trigger: {
      // the `status` variable does not exist if we're looking at `MC::Particle`
      bool has_status = const_cast<hipo::bank&>(particle_bank).getSchema().exists("status");
      // loop over ALL rows, not just filtered rows, since we don't want to accidentally pick the wrong lepton
      for(int row = 0; row < particle_bank.getRows(); row++) {
        if(particle_bank.getInt("pid", row) == o_beam_pdg) { // if beam PDG
          // check if in FD: use `status` if we have it, otherwise rough theta cut
          bool in_FD_trigger = false;
          if(has_status) {
            auto status   = particle_bank.getShort("status", row);
            in_FD_trigger = status > -3000 && status <= -2000; // trigger && in FD
          }
          else {
            ROOT::Math::XYZVector p(
                particle_bank.getFloat("px", row),
                particle_bank.getFloat("py", row),
                particle_bank.getFloat("pz", row));
            in_FD_trigger = p.theta() * TMath::RadToDeg() > o_theta_between_FD_and_FT; // rough theta cut
          }
          if(in_FD_trigger) {
            m_log->Trace("row {} is in FD trigger", row);
            double en = std::sqrt(
                std::pow(particle_bank.getFloat("px", row), 2) +
                std::pow(particle_bank.getFloat("py", row), 2) +
                std::pow(particle_bank.getFloat("pz", row), 2) +
                std::pow(o_beam_mass, 2));
            if(en > lepton_energy) { // select max-E
              lepton_row    = row;
              lepton_energy = en;
            }
          }
        }
      }
      break;
    }
    // ----------------------------------------------------------------------------------
    // use MC::Lund to find the lepton that has a beam parent
    // ----------------------------------------------------------------------------------
    case method_lepton_finder::lund_beam_daughter: {
      // find the beam lepton, assuming it has parent index == 0
      // loop over ALL rows, in case the user filtered out beam particles
      std::optional<int> beam_index = std::nullopt;
      for(int row = 0; row < particle_bank.getRows(); row++) {
        if(particle_bank.getInt("pid", row) == o_beam_pdg && particle_bank.getByte("parent", row) == 0) {
          beam_index = particle_bank.getByte("index", row);
          break;
          // FIXME: should we check if there are more than 1?
        }
      }
      // find the lepton with parent == beam lepton
      // loop over ALL rows, not just filtered rows, since we don't want to accidentally pick the wrong lepton
      if(beam_index.has_value()) {
        for(int row = 0; row < particle_bank.getRows(); row++) {
          if(particle_bank.getInt("pid", row) == o_beam_pdg && particle_bank.getByte("parent", row) == beam_index.value()) {
            lepton_row = row;
            break;
            // FIXME: should we check if there are more than 1?
          }
        }
      }
      else
        m_log->Debug("Failed to find beam lepton");
      // complain if lepton not found
      if(!lepton_row.has_value())
        m_log->Debug("Failed to find scattered lepton");
      break;
    }
    }

    // make sure `lepton_row` was not filtered out
    if(lepton_row.has_value()) {
      auto rowlist = particle_bank.getRowList();
      if(std::find(rowlist.begin(), rowlist.end(), lepton_row.value()) == rowlist.end())
        lepton_row = std::nullopt;
    }

    // return
    if(lepton_row.has_value())
      m_log->Debug("Found scattered lepton: row={}", lepton_row.value());
    else
      m_log->Debug("Scattered lepton not found");
    return lepton_row;
  }

  ///////////////////////////////////////////////////////////////////////////////

  concurrent_key_t InclusiveKinematics::PrepareEvent(int const runnum, double const beam_energy) const
  {
    m_log->Trace("calling PrepareEvent({})", runnum);
    if(o_runnum->NeedsHashing()) {
      std::hash<int> hash_ftn;
      auto hash_key = hash_ftn(runnum);
      if(!o_runnum->HasKey(hash_key))
        Reload(runnum, beam_energy, hash_key);
      return hash_key;
    }
    else {
      if(o_runnum->IsEmpty() || o_runnum->Load(0) != runnum)
        Reload(runnum, beam_energy, 0);
      return 0;
    }
  }

  ///////////////////////////////////////////////////////////////////////////////

  void InclusiveKinematics::Reload(int const runnum, double const user_beam_energy, concurrent_key_t key) const
  {
    std::lock_guard<std::mutex> const lock(m_mutex);
    m_log->Trace("-> calling Reload({}, {}, {})", runnum, user_beam_energy, key);
    o_runnum->Save(runnum, key);

    // parse config params
    auto beam_energy     = user_beam_energy < 0 ? m_rcdb->GetBeamEnergy(runnum) : user_beam_energy;
    auto beam_direction  = GetOptionVector<double>({"initial_state", GetConfig()->InRange("runs", runnum), "beam_direction"});
    auto target_particle = GetOptionScalar<std::string>({"initial_state", GetConfig()->InRange("runs", runnum), "target_particle"});

    // get the target mass and momentum
    double target_mass = -1;
    for(auto const& [pdg, name] : particle::name) {
      if(name == target_particle) {
        target_mass = particle::mass.at(pdg);
        break;
      }
    }
    if(target_mass < 0) {
      m_log->Error("Unknown target particle {:?}", target_particle);
      throw std::runtime_error("Reload failed");
    }
    double target_px = 0.0;
    double target_py = 0.0;
    double target_pz = 0.0;

    // get the beam momentum
    double beam_px, beam_py, beam_pz;
    if(beam_direction.size() != 3) {
      m_log->Error("Beam direction is not a 3-vector; assuming it is (0, 0, 1) instead");
      beam_direction = {0.0, 0.0, 1.0};
    }
    auto dir_mag = std::hypot(beam_direction[0], beam_direction[1], beam_direction[2]);
    auto beam_p  = std::sqrt(std::pow(beam_energy, 2) - std::pow(o_beam_mass, 2));
    if(dir_mag > 0) {
      beam_px = beam_direction[0] * beam_p / dir_mag;
      beam_py = beam_direction[1] * beam_p / dir_mag;
      beam_pz = beam_direction[2] * beam_p / dir_mag;
    }
    else {
      m_log->Error("Beam direction magnitude is not > 0");
      throw ::std::runtime_error("Reload failed");
    }

    // save the configuration
    m_log->Trace("-> Reloaded beam:   ({}, {}, {}, {})", beam_px, beam_py, beam_pz, o_beam_mass);
    m_log->Trace("-> Reloaded target: ({}, {}, {}, {})", target_px, target_py, target_pz, target_mass);
    o_beam_PxPyPzM->Save({beam_px, beam_py, beam_pz, o_beam_mass}, key);
    o_target_PxPyPzM->Save({target_px, target_py, target_pz, target_mass}, key);
  }

  ///////////////////////////////////////////////////////////////////////////////

  InclusiveKinematicsVars InclusiveKinematics::ComputeFromLepton(
      vector_element_t const lepton_px,
      vector_element_t const lepton_py,
      vector_element_t const lepton_pz,
      concurrent_key_t const key) const
  {
    InclusiveKinematicsVars result;

    m_log->Trace("Reconstruct inclusive kinematics from lepton with p=({}, {}, {}), key={}", lepton_px, lepton_py, lepton_pz, key);

    enum { px,
           py,
           pz,
           m };
    auto beam   = o_beam_PxPyPzM->Load(key);
    auto target = o_target_PxPyPzM->Load(key);

    ROOT::Math::PxPyPzMVector vec_beam(beam[px], beam[py], beam[pz], beam[m]);
    ROOT::Math::PxPyPzMVector vec_target(target[px], target[py], target[pz], target[m]);
    ROOT::Math::PxPyPzMVector vec_lepton(lepton_px, lepton_py, lepton_pz, beam[m]);

    auto vec_q     = vec_beam - vec_lepton;
    result.qx      = vec_q.Px();
    result.qy      = vec_q.Py();
    result.qz      = vec_q.Pz();
    result.qE      = vec_q.E();
    result.Q2      = -1 * vec_q.M2();
    result.x       = result.Q2 / (2 * vec_q.Dot(vec_target));
    result.y       = vec_target.Dot(vec_q) / vec_target.Dot(vec_beam);
    result.W       = (vec_beam + vec_target - vec_lepton).M();
    result.nu      = vec_target.Dot(vec_q) / target[m];
    result.beamPz  = beam[pz];
    result.targetM = target[m];

    m_log->Trace("Result: Q2={}  x={}  W={}", result.Q2, result.x, result.W);

    return result;
  }

  ///////////////////////////////////////////////////////////////////////////////

}
