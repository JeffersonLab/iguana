#include "Algorithm.h"
#include "iguana/algorithms/TypeDefs.h"
#include "iguana/services/YAMLReader.h"

#include <algorithm> // std::clamp
#include <cmath>     // std::sqrt
#include <string>
#include <vector>
#include <array>
#include <cstdlib>   // std::getenv
#include <optional>
#include <mutex>
#include <unordered_map>

namespace iguana::clas12 {

  // helper: does the banklist the framework gave us include a bank with this name?
  // note: bank::getSchema() is non-const in hipo4, so banklist is non-const here
  static bool banklist_has(hipo::banklist& banks, const char* name) {
    for (auto& b : banks) {
      if (b.getSchema().getName() == name) return true;
    }
    return false;
  }

  // try hard to retrieve a vector<double> from various YAML shapes/paths
  static std::optional<std::vector<double>>
  try_get_vec_d(const Algorithm* self, const std::string& key,
                std::initializer_list<std::string> prefixes = {})
  {
    if (!self || !self->GetConfig()) return std::nullopt;

    // 1) plain key (relative to algo root)
    try { return self->GetOptionVector<double>(key); } catch (...) {}

    // 2) with prefixes using path API or dotted/slashed keys
    for (auto const& p : prefixes) {
      try { return self->GetOptionVector<double>(key, YAMLReader::node_path_t{ p }); } catch (...) {}
      try { return self->GetOptionVector<double>(p + "." + key); } catch (...) {}
      try { return self->GetOptionVector<double>(p + "/" + key); } catch (...) {}
    }
    return std::nullopt;
  }

  // turn a flat vector into [a,b] windows
  static std::vector<std::pair<float,float>> to_windows_flat(const std::vector<double>& v)
  {
    std::vector<std::pair<float,float>> w;
    w.reserve(v.size() / 2);
    for (size_t i = 0; i + 1 < v.size(); i += 2)
      w.emplace_back(static_cast<float>(v[i]), static_cast<float>(v[i+1]));
    return w;
  }

  REGISTER_IGUANA_ALGORITHM(RGAFiducialFilter);

  // -------------------------
  // lifecycle
  // -------------------------

  void RGAFiducialFilter::Start(hipo::banklist& banks)
  {
    // Load this algorithm's YAML (safe if missing; we handle defaults)
    ParseYAMLConfig();

    // thread-safe params
    o_runnum         = ConcurrentParamFactory::Create<int>();
    o_cal_strictness = ConcurrentParamFactory::Create<int>();

    // --- strictness precedence: user setter > env var > YAML > default(1) ---
    if (!u_strictness_user.has_value()) {
      if (const char* s = std::getenv("IGUANA_RGAFID_STRICTNESS")) {
        try { u_strictness_user = std::clamp(std::stoi(s), 1, 3); } catch (...) {}
      }
    }
    if (!u_strictness_user.has_value()) {
      try {
        auto v = GetOptionVector<int>("strictness", YAMLReader::node_path_t{ "calorimeter" });
        if (!v.empty()) u_strictness_user = std::clamp(v.front(), 1, 3);
      } catch (...) { /* keep default */ }
    }
    if (!u_strictness_user.has_value()) u_strictness_user = 1;

    // Forward Tagger parameters from YAML (optional; defaults if missing)
    u_ft_params = FTParams{}; // default rmin=8.5, rmax=15.5, empty holes (see struct defaults)

    try {
      if (auto r = try_get_vec_d(this, "radius", { "forward_tagger" })) {
        if (r->size() >= 2) {
          float a = static_cast<float>((*r)[0]);
          float b = static_cast<float>((*r)[1]);
          u_ft_params.rmin = std::min(a, b);
          u_ft_params.rmax = std::max(a, b);
        }
      }

      // holes: prefer a flat list if present, otherwise try the nested key
      if (auto flat = try_get_vec_d(this, "holes_flat", { "forward_tagger" })) {
        for (size_t i = 0; i + 2 < flat->size(); i += 3)
          u_ft_params.holes.push_back({
            static_cast<float>((*flat)[i]),
            static_cast<float>((*flat)[i+1]),
            static_cast<float>((*flat)[i+2])
          });
      } else if (auto flat2 = try_get_vec_d(this, "holes", { "forward_tagger" })) {
        // also accept flattened "holes" for convenience
        for (size_t i = 0; i + 2 < flat2->size(); i += 3)
          u_ft_params.holes.push_back({
            static_cast<float>((*flat2)[i]),
            static_cast<float>((*flat2)[i+1]),
            static_cast<float>((*flat2)[i+2])
          });
      }
    } catch (...) {
      // keep defaults on any YAML read problem
    }

    // required banks
    b_particle = GetBankIndex(banks, "REC::Particle");
    b_config   = GetBankIndex(banks, "RUN::config");

    // optional banks
    if (banklist_has(banks, "REC::Calorimeter")) {
      b_calor = GetBankIndex(banks, "REC::Calorimeter");
      m_have_calor = true;
    } else {
      m_have_calor = false;
      m_log->Info("Optional bank 'REC::Calorimeter' not in banklist; calorimeter fiducials will be skipped.");
    }

    if (banklist_has(banks, "REC::ForwardTagger")) {
      b_ft = GetBankIndex(banks, "REC::ForwardTagger");
      m_have_ft = true;
    } else {
      m_have_ft = false;
      m_log->Info("Optional bank 'REC::ForwardTagger' not in banklist; FT fiducials will be skipped.");
    }
  }

  void RGAFiducialFilter::Run(hipo::banklist& banks) const
  {
    auto& particleBank = GetBank(banks, b_particle, "REC::Particle");
    auto& configBank   = GetBank(banks, b_config,   "RUN::config");

    // Optional banks (nullptr => skip those cuts)
    const hipo::bank* calBankPtr = m_have_calor ? &GetBank(banks, b_calor, "REC::Calorimeter") : nullptr;
    const hipo::bank* ftBankPtr  = m_have_ft    ? &GetBank(banks, b_ft,    "REC::ForwardTagger") : nullptr;

    // prepare per-event/per-run cache (loads YAML masks for this run)
    auto key = PrepareEvent(configBank.getInt("run", 0));

    // filter tracks in place
    particleBank.getMutableRowList().filter([this, calBankPtr, ftBankPtr, key](auto /*bank*/, auto row) {
      const int track_index = row;
      const bool accept = Filter(track_index, calBankPtr, ftBankPtr, key);
      return accept ? 1 : 0;
    });
  }

  void RGAFiducialFilter::Stop() { /* nothing */ }

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

    o_runnum->Save(runnum, key);

    // calorimeter strictness (from user/env/YAML -> clamped)
    int strictness = std::clamp(u_strictness_user.value_or(1), 1, 3);
    o_cal_strictness->Save(strictness, key);

    // build and cache masks per run (from YAML only)
    if (m_masks_by_run.find(runnum) == m_masks_by_run.end()) {
      m_masks_by_run.emplace(runnum, BuildCalMaskCache(runnum));
    }
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

      if (h.has_any) {
        const int strictness = GetCalStrictness(key);
        if (!PassCalStrictness(h, strictness)) return false;

        if (strictness >= 2) {
          if (!PassCalDeadPMTMasks(h, key)) return false;
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
      case 1: if (h.lw1 <  9.0f || h.lv1 <  9.0f) return false; break;  // electrons in BSAs
      case 2: if (h.lw1 < 13.5f || h.lv1 < 13.5f) return false; break;  // photons in BSAs
      case 3: if (h.lw1 < 18.0f || h.lv1 < 18.0f) return false; break;  // cross sections
      default: return false;
    }
    return true;
  }

  RGAFiducialFilter::MaskMap RGAFiducialFilter::BuildCalMaskCache(int runnum) const
  {
    MaskMap out;

    if (!GetConfig()) return out; // no YAML -> no dead-PMT masks

    auto inrange = GetConfig()->InRange("runs", runnum); // selects the right masks list item

    auto read_axis = [this,&inrange](int sector, const char* layer, const char* axis) -> std::vector<window_t> {
      const std::string sec = std::to_string(sector);

      // 1) preferred: axis: { cal_mask: [a,b,c,d,...] }
      try {
        YAMLReader::node_path_t p = { "calorimeter","masks",inrange,"sectors",sec,layer,axis };
        auto flat = GetOptionVector<double>("cal_mask", p);
        if (!flat.empty()) return to_windows_flat(flat);
      } catch (...) {}

      // 2) fallback: layer: { axis: [a,b,c,d,...] }  (flat)
      try {
        YAMLReader::node_path_t p = { "calorimeter","masks",inrange,"sectors",sec,layer };
        auto flat = GetOptionVector<double>(axis, p);
        if (!flat.empty()) return to_windows_flat(flat);
      } catch (...) {}

      // (Older list-of-lists form is intentionally not supported to keep the schema simple.)
      return {};
    };

    for (int s = 1; s <= 6; ++s) {
      SectorMasks sm;
      sm.pcal.lv  = read_axis(s, "pcal",  "lv");
      sm.pcal.lw  = read_axis(s, "pcal",  "lw");
      sm.pcal.lu  = read_axis(s, "pcal",  "lu");
      sm.ecin.lv  = read_axis(s, "ecin",  "lv");
      sm.ecin.lw  = read_axis(s, "ecin",  "lw");
      sm.ecin.lu  = read_axis(s, "ecin",  "lu");
      sm.ecout.lv = read_axis(s, "ecout", "lv");
      sm.ecout.lw = read_axis(s, "ecout", "lw");
      sm.ecout.lu = read_axis(s, "ecout", "lu");
      out.emplace(s, std::move(sm));
    }
    return out;
  }

  bool RGAFiducialFilter::PassCalDeadPMTMasks(const CalLayers& h, concurrent_key_t key) const
  {
    const int runnum = GetRunNum(key);

    // Guard accesses to m_masks_by_run for thread-safety w.r.t. Reload()
    std::lock_guard<std::mutex> const lock(m_mutex);

    auto it = m_masks_by_run.find(runnum);
    if (it == m_masks_by_run.end()) {
      // Should not happen if Reload() ran, but be defensive.
      it = m_masks_by_run.emplace(runnum, BuildCalMaskCache(runnum)).first;
    }
    const auto& m = it->second;

    auto itsec = m.find(h.sector);
    if (itsec == m.end()) return true;
    const auto& sm = itsec->second;

    auto in_any = [](float v, const std::vector<window_t>& wins){
      for (auto const& w : wins) if (v > w.first && v < w.second) return true;
      return false;
    };

    if (in_any(h.lv1, sm.pcal.lv) || in_any(h.lw1, sm.pcal.lw) || in_any(h.lu1, sm.pcal.lu)) return false;
    if (in_any(h.lv4, sm.ecin.lv) || in_any(h.lw4, sm.ecin.lw) || in_any(h.lu4, sm.ecin.lu)) return false;
    if (in_any(h.lv7, sm.ecout.lv)|| in_any(h.lw7, sm.ecout.lw)|| in_any(h.lu7, sm.ecout.lu)) return false;

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