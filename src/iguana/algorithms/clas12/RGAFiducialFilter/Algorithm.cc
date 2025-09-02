#include "Algorithm.h"
#include "iguana/algorithms/TypeDefs.h"

namespace iguana::clas12 {

  REGISTER_IGUANA_ALGORITHM(RGAFiducialFilter);

  // -------------------------
  // lifecycle
  // -------------------------

  void RGAFiducialFilter::Start(hipo::banklist& banks)
  {
    // parse YAML, allocate thread-safe per-run cache
    ParseYAMLConfig();
    o_runnum         = ConcurrentParamFactory::Create<int>();
    o_cal_strictness = ConcurrentParamFactory::Create<int>();

    // expected banks
    b_particle = GetBankIndex(banks, "REC::Particle");
    b_calor    = GetBankIndex(banks, "REC::Calorimeter");
    b_config   = GetBankIndex(banks, "RUN::config");
  }

  void RGAFiducialFilter::Run(hipo::banklist& banks) const
  {
    auto& particleBank = GetBank(banks, b_particle, "REC::Particle");
    auto& calorBank    = GetBank(banks, b_calor,    "REC::Calorimeter");
    auto& configBank   = GetBank(banks, b_config,   "RUN::config");

    // prepare per-event/per-run cache
    auto key = PrepareEvent(configBank.getInt("run", 0));

    // filter tracks in place
    particleBank.getMutableRowList().filter([this, &calorBank, key](auto bank, auto row) {
      const int track_index = row; // link to REC::Calorimeter via pindex
      const bool accept = Filter(track_index, calorBank, key);
      m_log->Debug("RGAFiducialFilter: track {} -> {}", track_index, accept ? "keep" : "drop");
      return accept ? 1 : 0;
    });
  }

  void RGAFiducialFilter::Stop()
  {
    // nothing to do
  }

  // -------------------------
  // event preparation
  // -------------------------

  concurrent_key_t RGAFiducialFilter::PrepareEvent(int runnum) const
  {
    if (o_runnum->NeedsHashing()) {
      std::hash<int> hash_ftn;
      auto key = hash_ftn(runnum);
      if (!o_runnum->HasKey(key)) Reload(runnum, key);
      return key;
    } else {
      if (o_runnum->IsEmpty() || o_runnum->Load(0) != runnum) Reload(runnum, 0);
      return 0;
    }
  }

  void RGAFiducialFilter::Reload(int runnum, concurrent_key_t key) const
  {
    std::lock_guard<std::mutex> const lock(m_mutex); // guard successive saves
    m_log->Trace("RGAFiducialFilter::Reload(run={}, key={})", runnum, key);

    // cache the run number
    o_runnum->Save(runnum, key);

    // read strictness (scalar) from YAML: clas12::RGAFiducialFilter -> calorimeter -> strictness
    int strictness = 1;
    try {
      auto v = GetOptionVector<int>("strictness", { "calorimeter" });
      if (!v.empty()) strictness = v.front();
    } catch (...) {
      // leave as default 1
    }
    if (strictness < 1) strictness = 1;
    if (strictness > 3) strictness = 3;
    o_cal_strictness->Save(strictness, key);
  }

  // -------------------------
  // core filter
  // -------------------------

  bool RGAFiducialFilter::Filter(int track_index,
                                 const hipo::bank& calBank,
                                 concurrent_key_t key) const
  {
    // collect calorimeter info linked to this track
    CalLayers h = CollectCalHitsForTrack(calBank, track_index);

    // if there is no calorimeter association, pass the track through
    if (!h.has_any) return true;

    // PCAL edge strictness (required at all levels)
    const int strictness = GetCalStrictness(key);
    if (!PassCalStrictness(h, strictness)) return false;

    // dead-PMT masks only for strictness >= 2
    const int runnum = GetRunNum(key);
    if (strictness >= 2) {
      if (!PassCalDeadPMTMasks(h, runnum)) return false;
    }

    return true;
  }

  // -------------------------
  // helpers
  // -------------------------

  RGAFiducialFilter::CalLayers
  RGAFiducialFilter::CollectCalHitsForTrack(const hipo::bank& calBank, int pindex)
  {
    CalLayers out;
    const int nrows = calBank.getRows();
    for (int i = 0; i < nrows; ++i) {
      if (calBank.getInt("pindex", i) != pindex) continue;

      out.has_any = true;
      out.sector  = calBank.getInt("sector", i);
      const int layer = calBank.getInt("layer", i);

      const float lv = calBank.getFloat("lv", i);
      const float lw = calBank.getFloat("lw", i);
      const float lu = calBank.getFloat("lu", i);

      if      (layer == 1) { out.lv1 = lv; out.lw1 = lw; out.lu1 = lu; }
      else if (layer == 4) { out.lv4 = lv; out.lw4 = lw; out.lu4 = lu; }
      else if (layer == 7) { out.lv7 = lv; out.lw7 = lw; out.lu7 = lu; }
    }
    return out;
  }

  bool RGAFiducialFilter::PassCalStrictness(const CalLayers& h, int strictness)
  {
    // PCAL-only edge cuts on (lv1, lw1)
    switch (strictness) {
      case 1: // recommended for electrons in BSAs
        if (h.lw1 < 9.0f || h.lv1 < 9.0f) return false;
        break;
      case 2: // recommended for photons in BSAs
        if (h.lw1 < 13.5f || h.lv1 < 13.5f) return false;
        break;
      case 3: // recommended for cross sections
        if (h.lw1 < 18.0f || h.lv1 < 18.0f) return false;
        break;
      default:
        return false;
    }
    return true;
  }

  bool RGAFiducialFilter::PassCalDeadPMTMasks(const CalLayers& h, int runnum) const
  {
    const std::string sectorStr = std::to_string(h.sector);

    // helper: safe fetch of vector<vector<double>> at a YAML path
    auto get2d = [this](const YAMLReader::node_path_t& path) -> std::vector<std::vector<double>> {
      try {
        return GetOptionVector<std::vector<double>>("cal_mask", path);
      } catch (...) {
        return {};
      }
    };

    // build a node_path_t:
    // { "calorimeter", "masks", InRange("runs", runnum), "sectors", "<sector>", "<layer>", "<axis>" }
    auto make_path = [this, &sectorStr, runnum](const char* layer, const char* axis) {
      YAMLReader::node_path_t np = {
        "calorimeter",
        "masks",
        GetConfig()->InRange("runs", runnum),
        "sectors",
        sectorStr,
        layer,
        axis
      };
      return np;
    };

    const auto pcal_lv  = get2d(make_path("pcal",  "lv"));
    const auto pcal_lw  = get2d(make_path("pcal",  "lw"));
    const auto pcal_lu  = get2d(make_path("pcal",  "lu"));

    const auto ecin_lv  = get2d(make_path("ecin",  "lv"));
    const auto ecin_lw  = get2d(make_path("ecin",  "lw"));
    const auto ecin_lu  = get2d(make_path("ecin",  "lu"));

    const auto ecout_lv = get2d(make_path("ecout", "lv"));
    const auto ecout_lw = get2d(make_path("ecout", "lw"));
    const auto ecout_lu = get2d(make_path("ecout", "lu"));

    auto toWindows = [](const std::vector<std::vector<double>>& vv) {
      std::vector<std::pair<float,float>> w;
      w.reserve(vv.size());
      for (auto const& r : vv) {
        if (r.size() >= 2) w.emplace_back(static_cast<float>(r[0]), static_cast<float>(r[1]));
      }
      return w;
    };

    auto in_any = [](float v, const std::vector<std::pair<float,float>>& wins) {
      for (auto const& w : wins) if (v > w.first && v < w.second) return true;
      return false;
    };

    // Apply windows: any hit inside any forbidden window -> reject
    if (in_any(h.lv1, toWindows(pcal_lv)) || in_any(h.lw1, toWindows(pcal_lw)) || in_any(h.lu1, toWindows(pcal_lu))) return false;
    if (in_any(h.lv4, toWindows(ecin_lv)) || in_any(h.lw4, toWindows(ecin_lw)) || in_any(h.lu4, toWindows(ecin_lu))) return false;
    if (in_any(h.lv7, toWindows(ecout_lv))|| in_any(h.lw7, toWindows(ecout_lw))|| in_any(h.lu7, toWindows(ecout_lu))) return false;

    return true;
  }

  // -------------------------
  // accessors
  // -------------------------

  int RGAFiducialFilter::GetRunNum(concurrent_key_t key) const
  {
    return o_runnum->Load(key);
  }

  int RGAFiducialFilter::GetCalStrictness(concurrent_key_t key) const
  {
    return o_cal_strictness->Load(key);
  }

} // namespace iguana::clas12