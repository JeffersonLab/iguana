#include "Algorithm.h"
#include "iguana/algorithms/TypeDefs.h"
#include "iguana/services/YAMLReader.h"

#include <algorithm> // std::clamp
#include <cmath>     // std::sqrt
#include <string>
#include <vector>

namespace iguana::clas12 {

  // helper: does the banklist include a bank with this schema name?
  static bool banklist_has(const hipo::banklist& banks, const char* name) {
    for (const auto& b : banks) {
      if (b.getSchema().getName() == name) return true;
    }
    return false;
  }

  REGISTER_IGUANA_ALGORITHM(RGAFiducialFilter);

  // -------------------------
  // lifecycle
  // -------------------------

  void RGAFiducialFilter::Start(hipo::banklist& banks)
  {
    // Load YAML for this algorithm
    ParseYAMLConfig();

    // allocate per-run concurrent cache
    o_runnum         = ConcurrentParamFactory::Create<int>();
    o_cal_strictness = ConcurrentParamFactory::Create<int>();

    // required (must be requested with -b on the command line)
    b_particle = GetBankIndex(banks, "REC::Particle");
    b_config   = GetBankIndex(banks, "RUN::config");

    // optional: Calorimeter
    if (banklist_has(banks, "REC::Calorimeter")) {
      b_calor = GetBankIndex(banks, "REC::Calorimeter");
      m_have_calor = true;
    } else {
      m_have_calor = false;
      m_log->Info("Optional bank 'REC::Calorimeter' not in banklist; calorimeter fiducials will be skipped.");
    }

    // optional: ForwardTagger
    if (banklist_has(banks, "REC::ForwardTagger")) {
      b_ft = GetBankIndex(banks, "REC::ForwardTagger");
      m_have_ft = true;
    } else {
      m_have_ft = false;
      m_log->Info("Optional bank 'REC::ForwardTagger' not in banklist; FT fiducials will be skipped.");
    }

    // Read forward_tagger params from YAML ONCE (keep compiled defaults if absent)
    // Path is: clas12::RGAFiducialFilter -> forward_tagger -> {radius, holes}
    try {
      {
        auto r = GetOptionVector<double>("radius", { "forward_tagger" });
        if (r.size() >= 2) {
          float a = static_cast<float>(r[0]);
          float b = static_cast<float>(r[1]);
          u_ft_params.rmin = std::min(a, b);
          u_ft_params.rmax = std::max(a, b);
        }
      }
      {
        auto flat = GetOptionVector<double>("holes", { "forward_tagger" });
        if (!flat.empty()) {
          std::vector<std::array<float,3>> holes;
          holes.reserve(flat.size()/3);
          for (size_t i = 0; i + 2 < flat.size(); i += 3) {
            holes.push_back({
              static_cast<float>(flat[i]),
              static_cast<float>(flat[i+1]),
              static_cast<float>(flat[i+2])
            });
          }
          if (!holes.empty()) u_ft_params.holes = std::move(holes);
        }
      }
    } catch (...) {
      // Keep defaults: rmin=8.5, rmax=15.5, holes={}
      // If the YAML keys are missing and GetOptionVector logs once, it happens only here.
    }
  }

  void RGAFiducialFilter::Run(hipo::banklist& banks) const
  {
    auto& particleBank = GetBank(banks, b_particle, "REC::Particle");
    auto& configBank   = GetBank(banks, b_config,   "RUN::config");

    // Pointers for optional banks (null means skip that detector's cuts)
    const hipo::bank* calBankPtr = nullptr;
    const hipo::bank* ftBankPtr  = nullptr;

    if (m_have_calor) {
      calBankPtr = &GetBank(banks, b_calor, "REC::Calorimeter");
    }
    if (m_have_ft) {
      ftBankPtr = &GetBank(banks, b_ft, "REC::ForwardTagger");
    }

    // Prepare per-event/per-run cache
    auto key = PrepareEvent(configBank.getInt("run", 0));

    // Filter tracks in place
    particleBank.getMutableRowList().filter([this, calBankPtr, ftBankPtr, key](auto /*bank*/, auto row) {
      const int track_index = row;
      const bool accept = Filter(track_index, calBankPtr, ftBankPtr, key);
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
    std::lock_guard<std::mutex> const lock(m_mutex);
    m_log->Trace("RGAFiducialFilter::Reload(run={}, key={})", runnum, key);

    // cache the run number
    o_runnum->Save(runnum, key);

    // calorimeter strictness (1..3), user may have set it via SetStrictness
    int strictness = std::clamp(u_strictness_user.value_or(1), 1, 3);
    o_cal_strictness->Save(strictness, key);

    // Note: FT YAML is read once in Start(), not per run.
  }

  // -------------------------
  // user setter
  // -------------------------

  void RGAFiducialFilter::SetStrictness(int strictness)
  {
    std::lock_guard<std::mutex> const lock(m_mutex);
    u_strictness_user = std::clamp(strictness, 1, 3);
  }

  // -------------------------
  // core filter
  // -------------------------

  bool RGAFiducialFilter::Filter(int track_index,
                                 const hipo::bank* calBank,
                                 const hipo::bank* ftBank,
                                 concurrent_key_t key) const
  {
    // Calorimeter: apply only if we have a cal bank
    if (calBank != nullptr) {
      CalLayers h = CollectCalHitsForTrack(*calBank, track_index);

      // pass if no cal association OR if it passes strictness (plus masks when >=2)
      if (h.has_any) {
        const int strictness = GetCalStrictness(key);
        if (!PassCalStrictness(h, strictness)) return false;

        const int runnum = GetRunNum(key);
        if (strictness >= 2) {
          if (!PassCalDeadPMTMasks(h, runnum)) return false;
        }
      }
    }

    // Forward Tagger: apply only if we have an FT bank
    if (!PassFTFiducial(track_index, ftBank)) return false;

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

    auto get_flat = [this](const YAMLReader::node_path_t& path) -> std::vector<double> {
      try {
        return GetOptionVector<double>("cal_mask", path);
      } catch (...) {
        return {};
      }
    };

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

  bool RGAFiducialFilter::PassFTFiducial(int track_index, const hipo::bank* ftBank) const
  {
    // If the FT bank is not present for this file/event, we skip FT cuts (pass-through).
    if (ftBank == nullptr) return true;

    const int nrows = ftBank->getRows();
    for (int i = 0; i < nrows; ++i) {
      if (ftBank->getInt("pindex", i) != track_index) continue;

      const double x = ftBank->getFloat("x", i);
      const double y = ftBank->getFloat("y", i);
      const double r = std::sqrt(x*x + y*y);

      // radial window
      if (r < u_ft_params.rmin) return false;
      if (r > u_ft_params.rmax) return false;

      // holes (circles to exclude)
      for (auto const& h : u_ft_params.holes) {
        const double hr = h[0], cx = h[1], cy = h[2];
        const double d  = std::sqrt((x - cx)*(x - cx) + (y - cy)*(y - cy));
        if (d < hr) return false;
      }

      // this FT association passes
      return true;
    }

    // No FT association for this track in this event -> pass-through
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