#include "Algorithm.h"
#include "iguana/algorithms/TypeDefs.h"
#include "iguana/services/YAMLReader.h"

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
      strictness = GetOption<int>("strictness", {"calorimeter"});
    } catch(...) {
      /* keep default 1 */
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

    // Build a node_path like:
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

    // Safe fetch: returns a flat vector<double> (length is 2*N for N windows) or empty if missing
    auto get_flat = [this](const YAMLReader::node_path_t& path) -> std::vector<double> {
      try {
        return GetOptionVector<double>("cal_mask", path);
      } catch (...) {
        return {};
      }
    };

    // Convert flat [a,b,c,d,...] -> vector<pair<float,float>> windows
    auto to_windows = [](const std::vector<double>& v) {
      std::vector<std::pair<float,float>> w;
      w.reserve(v.size() / 2);
      for (size_t i = 0; i + 1 < v.size(); i += 2) {
        w.emplace_back(static_cast<float>(v[i]), static_cast<float>(v[i+1]));
      }
      return w;
    };

    auto in_any = [](float val, const std::vector<std::pair<float,float>>& wins) {
      for (auto const& w : wins) if (val > w.first && val < w.second) return true;
      return false;
    };

    // Load and check all windows for this sector/layer/axis
    const auto pcal_lv  = to_windows(get_flat(make_path("pcal",  "lv")));
    const auto pcal_lw  = to_windows(get_flat(make_path("pcal",  "lw")));
    const auto pcal_lu  = to_windows(get_flat(make_path("pcal",  "lu")));

    const auto ecin_lv  = to_windows(get_flat(make_path("ecin",  "lv")));
    const auto ecin_lw  = to_windows(get_flat(make_path("ecin",  "lw")));
    const auto ecin_lu  = to_windows(get_flat(make_path("ecin",  "lu")));

    const auto ecout_lv = to_windows(get_flat(make_path("ecout", "lv")));
    const auto ecout_lw = to_windows(get_flat(make_path("ecout", "lw")));
    const auto ecout_lu = to_windows(get_flat(make_path("ecout", "lu")));

    if (in_any(h.lv1, pcal_lv) || in_any(h.lw1, pcal_lw) || in_any(h.lu1, pcal_lu)) return false;
    if (in_any(h.lv4, ecin_lv) || in_any(h.lw4, ecin_lw) || in_any(h.lu4, ecin_lu)) return false;
    if (in_any(h.lv7, ecout_lv)|| in_any(h.lw7, ecout_lw)|| in_any(h.lu7, ecout_lu)) return false;

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