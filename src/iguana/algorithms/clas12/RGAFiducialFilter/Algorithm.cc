// Algorithm.cc (drop the old body and paste this)

#include "Algorithm.h"
#include "iguana/services/YAMLReader.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <limits>

namespace iguana::clas12 {

REGISTER_IGUANA_ALGORITHM(RGAFiducialFilter);

// env helpers
bool RGAFiducialFilter::EnvOn(const char* name) {
  if (const char* s = std::getenv(name)) {
    std::string v{s}; return v=="1"||v=="true"||v=="TRUE";
  }
  return false;
}
int RGAFiducialFilter::EnvInt(const char* name, int def) {
  if (const char* s = std::getenv(name)) { try { return std::stoi(s); } catch (...) {} }
  return def;
}

static bool banklist_has(hipo::banklist& banks, const char* name) {
  for (auto& b : banks) if (b.getSchema().getName() == name) return true;
  return false;
}

void RGAFiducialFilter::SetStrictness(int s) {
  m_strictness = std::clamp(s, 1, 3);
  m_strictness_overridden = true;
}

void RGAFiducialFilter::LoadConfigFromYAML()
{
  ParseYAMLConfig();
  if (!GetConfig()) return;

  // strictness (only if not programmatically overridden)
  if (!m_strictness_overridden) {
    try {
      auto v = GetOptionVector<int>("calorimeter.strictness");
      if (!v.empty()) m_strictness = std::clamp(v.front(), 1, 3);
    } catch (...) {}
  }

  // FT radius
  try {
    auto r = GetOptionVector<double>("forward_tagger.radius");
    if (r.size() >= 2) {
      float a = static_cast<float>(r[0]);
      float b = static_cast<float>(r[1]);
      m_ft.rmin = std::min(a, b);
      m_ft.rmax = std::max(a, b);
    }
  } catch (...) {}

  // FT holes
  try {
    auto flat = GetOptionVector<double>("forward_tagger.holes_flat");
    if (!flat.empty()) {
      m_ft.holes.clear();
      for (size_t i=0; i+2<flat.size(); i+=3)
        m_ft.holes.push_back({ (float)flat[i], (float)flat[i+1], (float)flat[i+2] });
    }
  } catch (...) {}
}

void RGAFiducialFilter::Start(hipo::banklist& banks)
{
  // Debug knobs
  dbg_on     = EnvOn("IGUANA_RGAFID_DEBUG");
  dbg_ft     = EnvOn("IGUANA_RGAFID_DEBUG_FT");
  dbg_events = EnvInt("IGUANA_RGAFID_DEBUG_EVENTS", 0);
  if (dbg_on) m_log->Info("[RGAFID][DEBUG] ft={}, events={}", dbg_ft, dbg_events);

  // defaults for FT holes (in case YAML is absent)
  if (m_ft.holes.empty()) {
    const float H[] = {
      1.60f, -8.42f,  9.89f,
      1.60f, -9.89f, -5.33f,
      2.30f, -6.15f, -13.00f,
      2.00f,  3.70f, -6.50f
    };
    for (size_t i=0; i+2<std::size(H); i+=3)
      m_ft.holes.push_back({H[i],H[i+1],H[i+2]});
  }

  // Load YAML from the same directory (like ZVertexFilter)
  LoadConfigFromYAML();
  if (dbg_on) {
    m_log->Info("[RGAFID] strictness = {}", m_strictness);
    m_log->Info("[RGAFID][FT] rmin={:.3f} rmax={:.3f} holes={}", m_ft.rmin, m_ft.rmax, m_ft.holes.size());
  }

  // required banks
  b_particle = GetBankIndex(banks, "REC::Particle");
  b_config   = GetBankIndex(banks, "RUN::config");

  // optional banks
  if (banklist_has(banks, "REC::Calorimeter")) { b_calor = GetBankIndex(banks, "REC::Calorimeter"); m_have_calor=true; }
  if (banklist_has(banks, "REC::ForwardTagger")) { b_ft = GetBankIndex(banks, "REC::ForwardTagger"); m_have_ft=true; }
}

void RGAFiducialFilter::Run(hipo::banklist& banks) const
{
  auto& particleBank = GetBank(banks, b_particle, "REC::Particle");
  const hipo::bank* calBankPtr = m_have_calor ? &GetBank(banks, b_calor, "REC::Calorimeter") : nullptr;
  const hipo::bank* ftBankPtr  = m_have_ft    ? &GetBank(banks, b_ft,    "REC::ForwardTagger") : nullptr;

  particleBank.getMutableRowList().filter([this, calBankPtr, ftBankPtr](auto, auto row) {
    const int track_index = row;

    // PCAL edge cuts
    if (calBankPtr) {
      std::vector<CalHit> pcal_hits;
      CollectPCALHitsForTrack(*calBankPtr, track_index, pcal_hits);
      if (!pcal_hits.empty() && !PassPCalEdgeCuts(pcal_hits)) return 0;
    }

    // FT annulus + holes
    if (!PassFTFiducial(track_index, ftBankPtr)) return 0;

    return 1;
  });
}

void RGAFiducialFilter::CollectPCALHitsForTrack(const hipo::bank& cal, int pindex,
                                                std::vector<CalHit>& out_hits)
{
  const int n = cal.getRows();
  for (int i=0;i<n;++i) {
    if (cal.getInt("pindex", i) != pindex) continue;
    if (cal.getInt("layer",  i) != 1)      continue; // PCAL only
    CalHit h;
    h.layer  = 1;
    h.sector = cal.getInt("sector", i);
    h.lv     = cal.getFloat("lv", i);
    h.lw     = cal.getFloat("lw", i);
    h.lu     = cal.getFloat("lu", i);
    out_hits.push_back(h);
  }
}

bool RGAFiducialFilter::PassPCalEdgeCuts(const std::vector<CalHit>& hits) const
{
  // use minima across PCAL hits
  float min_lv = std::numeric_limits<float>::infinity();
  float min_lw = std::numeric_limits<float>::infinity();
  for (auto const& h : hits) {
    if (h.lv < min_lv) min_lv = h.lv;
    if (h.lw < min_lw) min_lw = h.lw;
  }

  const float thr = (m_strictness==1 ?  9.0f :
                    (m_strictness==2 ? 13.5f : 18.0f));

  return !(min_lv < thr || min_lw < thr);
}

bool RGAFiducialFilter::PassFTFiducial(int track_index, const hipo::bank* ftBank) const
{
  if (!ftBank) return true;

  const int n = ftBank->getRows();
  for (int i=0;i<n;++i) {
    if (ftBank->getInt("pindex", i) != track_index) continue;

    const double x = ftBank->getFloat("x", i);
    const double y = ftBank->getFloat("y", i);
    const double r = std::sqrt(x*x + y*y);

    if (dbg_ft && dbg_events>0) {
      m_log->Info("[RGAFID][FT] trk={} x={:.2f} y={:.2f} r={:.2f} rwin=[{:.2f},{:.2f}]",
                  track_index, x, y, r, m_ft.rmin, m_ft.rmax);
    }

    if (r < m_ft.rmin || r > m_ft.rmax) return false;
    for (auto const& h : m_ft.holes) {
      const double d = std::sqrt((x-h[1])*(x-h[1]) + (y-h[2])*(y-h[2]));
      if (d < h[0]) return false;
    }
    return true; // first associated FT row decides
  }
  return true; // no FT association -> pass
}

} // namespace iguana::clas12