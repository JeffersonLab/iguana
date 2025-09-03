#include "Algorithm.h"
#include "iguana/algorithms/TypeDefs.h"
#include "iguana/services/YAMLReader.h"

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>
#include <array>
#include <cstdlib>
#include <optional>

namespace iguana::clas12 {

  static bool banklist_has(hipo::banklist& banks, const char* name) {
    for (auto& b : banks) if (b.getSchema().getName() == name) return true;
    return false;
  }

  REGISTER_IGUANA_ALGORITHM(RGAFiducialFilter);

  // -------------------------
  // lifecycle
  // -------------------------

  void RGAFiducialFilter::Start(hipo::banklist& banks)
  {
    ParseYAMLConfig();
    // show which YAML we actually loaded (see "A" in my message)
    try {
      m_log->Info("RGAFid YAML marker = {}", GetOptionScalar<std::string>("__marker__"));
    } catch (...) {
      m_log->Warn("RGAFid YAML marker not found — likely not reading your edited Config.yaml");
    }

    o_runnum         = ConcurrentParamFactory::Create<int>();
    o_cal_strictness = ConcurrentParamFactory::Create<int>();
    o_cal_masks      = ConcurrentParamFactory::Create<MaskMap>();

    // strictness precedence: user setter > env var > YAML > default(1)
    if (!u_strictness_user.has_value()) {
      if (const char* s = std::getenv("IGUANA_RGAFID_STRICTNESS")) {
        try { u_strictness_user = std::clamp(std::stoi(s), 1, 3); } catch (...) {}
      }
    }
    if (!u_strictness_user.has_value()) {
      try {
        auto v = GetOptionVector<int>("strictness", YAMLReader::node_path_t{ "calorimeter" });
        if (!v.empty()) u_strictness_user = std::clamp(v.front(), 1, 3);
      } catch (...) { /* default later */ }
    }

    // FT params: read once (quietly) from forward_tagger
    try {
      auto r = GetOptionVector<double>("radius", YAMLReader::node_path_t{ "forward_tagger" });
      if (r.size() >= 2) {
        float a = static_cast<float>(r[0]), b = static_cast<float>(r[1]);
        u_ft_params.rmin = std::min(a, b);
        u_ft_params.rmax = std::max(a, b);
      }
    } catch (...) {}
    try {
      auto flat = GetOptionVector<double>("holes_flat", YAMLReader::node_path_t{ "forward_tagger" });
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
    } catch (...) {
      // optional nested form: we only try it if holes_flat wasn't present
      try {
        auto nested = GetOptionVector<double>("holes", YAMLReader::node_path_t{ "forward_tagger" });
        // many YAML readers won't flatten list-of-lists to doubles; ignore if it fails
      } catch (...) {}
    }

    // required
    b_particle = GetBankIndex(banks, "REC::Particle");
    b_config   = GetBankIndex(banks, "RUN::config");

    // optional
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

    const hipo::bank* calBankPtr = m_have_calor ? &GetBank(banks, b_calor, "REC::Calorimeter") : nullptr;
    const hipo::bank* ftBankPtr  = m_have_ft    ? &GetBank(banks, b_ft,    "REC::ForwardTagger") : nullptr;

    auto key = PrepareEvent(configBank.getInt("run", 0));

    particleBank.getMutableRowList().filter([this, calBankPtr, ftBankPtr, key](auto /*bank*/, auto row) {
      const bool accept = Filter(row, calBankPtr, ftBankPtr, key);
      return accept ? 1 : 0;
    });
  }

  void RGAFiducialFilter::Stop() { }

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

    const int strictness = std::clamp(u_strictness_user.value_or(1), 1, 3);
    o_cal_strictness->Save(strictness, key);

    // build and cache masks once per run
    o_cal_masks->Save(BuildCalMaskCache(runnum), key);
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
    if (calBank != nullptr) {
      CalLayers h = CollectCalHitsForTrack(*calBank, track_index);

      if (h.has_any) {
        if (!PassCalStrictness(h, GetCalStrictness(key))) return false;

        if (GetCalStrictness(key) >= 2) {
          if (!PassCalDeadPMTMasks(h, key)) return false;
        }
      }
    }

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
    switch (strictness) {
      case 1: if (h.lw1 <  9.0f || h.lv1 <  9.0f) return false; break;
      case 2: if (h.lw1 < 13.5f || h.lv1 < 13.5f) return false; break;
      case 3: if (h.lw1 < 18.0f || h.lv1 < 18.0f) return false; break;
      default: return false;
    }
    return true;
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

  RGAFiducialFilter::MaskMap RGAFiducialFilter::BuildCalMaskCache(int runnum) const
  {
    MaskMap out;

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

      // (Older list-of-lists form is not attempted here to keep things quiet.)
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
    const auto& m = o_cal_masks->Load(key);
    auto it = m.find(h.sector);
    if (it == m.end()) return true;
    const auto& sm = it->second;

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
    if (ftBank == nullptr) return true;

    const int nrows = ftBank->getRows();
    for (int i = 0; i < nrows; ++i) {
      if (ftBank->getInt("pindex", i) != track_index) continue;

      const double x = ftBank->getFloat("x", i);
      const double y = ftBank->getFloat("y", i);
      const double r = std::sqrt(x*x + y*y);

      if (r < u_ft_params.rmin) return false;
      if (r > u_ft_params.rmax) return false;

      for (auto const& h : u_ft_params.holes) {
        const double d = std::sqrt((x - h[1])*(x - h[1]) + (y - h[2])*(y - h[2]));
        if (d < h[0]) return false;
      }
      return true;
    }
    return true; // no FT association -> pass
  }

  // -------------------------
  // accessors
  // -------------------------

  int RGAFiducialFilter::GetRunNum(concurrent_key_t key) const { return o_runnum->Load(key); }
  int RGAFiducialFilter::GetCalStrictness(concurrent_key_t key) const { return o_cal_strictness->Load(key); }

} // namespace iguana::clas12