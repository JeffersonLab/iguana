#include "Algorithm.h"
#include "iguana/services/YAMLReader.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <limits>

namespace iguana::clas12 {

REGISTER_IGUANA_ALGORITHM(RGAFiducialFilter);

static bool banklist_has(hipo::banklist& banks, const char* name) {
  for (auto& b : banks) if (b.getSchema().getName() == name) return true;
  return false;
}

void RGAFiducialFilter::LoadConfigFromYAML()
{
  ParseYAMLConfig();
  if (!GetConfig()) return;

  try {
    auto v = GetOptionVector<int>("calorimeter.strictness");
    if (!v.empty()) m_strictness = std::clamp(v.front(), 1, 3);
  } catch (...) {}

  try {
    auto r = GetOptionVector<double>("forward_tagger.radius");
    if (r.size() >= 2) {
      float a = static_cast<float>(r[0]);
      float b = static_cast<float>(r[1]);
      m_ft.rmin = std::min(a, b);
      m_ft.rmax = std::max(a, b);
    }
  } catch (...) {}

  try {
    auto flat = GetOptionVector<double>("forward_tagger.holes_flat");
    m_ft.holes.clear();
    for (size_t i=0; i+2<flat.size(); i+=3)
      m_ft.holes.push_back({ (float)flat[i], (float)flat[i+1], (float)flat[i+2] });
  } catch (...) {}
}

void RGAFiducialFilter::SetStrictness(int s) { m_strictness = std::clamp(s, 1, 3); }

void RGAFiducialFilter::Start(hipo::banklist& banks)
{
  // Debug knobs
  dbg_on     = EnvOn("IGUANA_RGAFID_DEBUG");
  dbg_masks  = false;                 // masks removed
  dbg_ft     = EnvOn("IGUANA_RGAFID_DEBUG_FT");
  dbg_events = EnvInt("IGUANA_RGAFID_DEBUG_EVENTS", 0);

  if (dbg_on) {
    m_log->Info("[RGAFID][DEBUG] enabled. ft={}, events={}", dbg_ft, dbg_events);
  }

  // === Load YAML from the SAME directory as this algorithm (like ZVertexFilter) ===
  // The framework search order already includes this algorithm's source dir.
  ParseYAMLConfig();
  if (dbg_on) m_log->Info("[RGAFID] YAML {}",
                          GetConfig() ? "found (using same-dir Config.yaml)" : "not found (using defaults)");

  // concurrent params
  o_runnum         = ConcurrentParamFactory::Create<int>();
  o_cal_strictness = ConcurrentParamFactory::Create<int>();

  // ---- Strictness precedence: SetStrictness() > YAML > default(1) ----
  if (!u_strictness_user.has_value()) {
    try {
      // YAML: clas12::RGAFiducialFilter: { calorimeter: { strictness: [1|2|3] } }
      auto v = GetOptionVector<int>("calorimeter.strictness");
      if (!v.empty()) u_strictness_user = std::clamp(v.front(), 1, 3);
    } catch (...) { /* keep default */ }
  }
  if (!u_strictness_user.has_value()) u_strictness_user = 1;  // default

  if (dbg_on) m_log->Info("[RGAFID] strictness (initial) = {}", *u_strictness_user);

  // ---- FT params: defaults first; optional YAML override if present ----
  u_ft_params = FTParams{}; // defaults rmin=8.5, rmax=15.5 and 4 holes
  try {
    auto rvec = GetOptionVector<double>("forward_tagger.radius");
    if (rvec.size() >= 2) {
      float a = static_cast<float>(rvec[0]);
      float b = static_cast<float>(rvec[1]);
      u_ft_params.rmin = std::min(a, b);
      u_ft_params.rmax = std::max(a, b);
    }
  } catch (...) { /* keep defaults */ }

  try {
    auto flat = GetOptionVector<double>("forward_tagger.holes_flat");
    if (!flat.empty()) {
      u_ft_params.holes.clear();
      for (size_t i = 0; i + 2 < flat.size(); i += 3) {
        u_ft_params.holes.push_back({
          static_cast<float>(flat[i]),
          static_cast<float>(flat[i+1]),
          static_cast<float>(flat[i+2])
        });
      }
    }
  } catch (...) { /* keep defaults */ }

  if (dbg_on || dbg_ft) DumpFTParams();

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

void RGAFiducialFilter::Reload(int runnum, concurrent_key_t key) const
{
  std::lock_guard<std::mutex> lock(m_mutex);
  o_runnum->Save(runnum, key);

  // If SetStrictness() was called programmatically, keep that;
  // otherwise use whatever YAML gave us earlier (already in u_strictness_user).
  const int strict = std::clamp(u_strictness_user.value_or(1), 1, 3);
  o_cal_strictness->Save(strict, key);

  if (dbg_on) {
    m_log->Info("[RGAFID][Reload] run={} key={} strictness={}", runnum, (uint64_t)key, strict);
  }
}

bool RGAFiducialFilter::Filter(int track_index,
                               const hipo::bank* calBank,
                               const hipo::bank* ftBank,
                               concurrent_key_t key) const
{
  // --- Calorimeter: PCAL edge cut only (no dead-PMT masks) ---
  if (calBank != nullptr) {
    CalLayers h = CollectCalHitsForTrack(*calBank, track_index);
    if (h.has_any) {
      const int strictness = GetCalStrictness(key);
      if (!PassCalStrictness(h, strictness)) {
        ++c_fail_edge;
        if (dbg_on && g_dbg_events_seen.load() < dbg_events) {
          float min_lv = std::numeric_limits<float>::infinity();
          float min_lw = std::numeric_limits<float>::infinity();
          for (const auto& hit : h.L1) {
            if (hit.lv < min_lv) min_lv = hit.lv;
            if (hit.lw < min_lw) min_lw = hit.lw;
          }
          m_log->Info("[RGAFID][CAL] track={} strictness={} -> edge FAIL (min lv1={:.1f}, min lw1={:.1f})",
                      track_index, strictness,
                      std::isinf(min_lv)?0.f:min_lv, std::isinf(min_lw)?0.f:min_lw);
        }
        return false;
      }
    }
  }

  // --- Forward Tagger: circle window + holes ---
  if (!PassFTFiducial(track_index, ftBank)) {
    ++c_fail_ft;
    if (dbg_on && g_dbg_events_seen.load() < dbg_events) {
      m_log->Info("[RGAFID][FT] track={} -> FT FAIL", track_index);
    }
    return false;
  }

  ++c_pass;
  return true;
}

} // namespace iguana::clas12