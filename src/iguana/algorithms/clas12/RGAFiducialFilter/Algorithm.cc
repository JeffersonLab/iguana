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
  if (const char* d = std::getenv("IGUANA_RGAFID_DEBUG"))
    m_dbg = (std::string(d)=="1" || std::string(d)=="true" || std::string(d)=="TRUE");

  LoadConfigFromYAML();

  b_particle = GetBankIndex(banks, "REC::Particle");
  b_config   = GetBankIndex(banks, "RUN::config");
  if (banklist_has(banks, "REC::Calorimeter")) { b_calor = GetBankIndex(banks, "REC::Calorimeter"); m_have_calor=true; }
  if (banklist_has(banks, "REC::ForwardTagger")) { b_ft = GetBankIndex(banks, "REC::ForwardTagger"); m_have_ft=true; }

  if (m_dbg) m_log->Info("[RGAFID] Start: strictness={}  FT r=[{:.1f},{:.1f}] holes={}",
                         m_strictness, m_ft.rmin, m_ft.rmax, m_ft.holes.size());
}

void RGAFiducialFilter::Run(hipo::banklist& banks) const
{
  auto& particle = GetBank(banks, b_particle, "REC::Particle");
  const hipo::bank* cal = m_have_calor ? &GetBank(banks, b_calor, "REC::Calorimeter") : nullptr;
  const hipo::bank* ft  = m_have_ft    ? &GetBank(banks, b_ft,    "REC::ForwardTagger") : nullptr;
  (void)GetBank(banks, b_config, "RUN::config");

  particle.getMutableRowList().filter([this, cal, ft](auto, auto row)->int {
    if (cal) {
      std::vector<CalHit> pcal_hits;
      CollectPCALHitsForTrack(*cal, row, pcal_hits);
      if (!pcal_hits.empty() && !PassPCalEdgeCuts(pcal_hits)) return 0;
    }
    if (!PassFTFiducial(row, ft)) return 0;
    return 1;
  });
}

void RGAFiducialFilter::Stop() {}

void RGAFiducialFilter::CollectPCALHitsForTrack(const hipo::bank& cal, int pindex,
                                                std::vector<CalHit>& out_hits)
{
  out_hits.clear();
  const int n = cal.getRows();
  for (int i=0;i<n;++i) {
    if (cal.getInt("pindex", i) != pindex) continue;
    if (cal.getInt("layer",  i) != 1)     continue; // PCAL only
    CalHit h;
    h.sector = cal.getInt ("sector", i);
    h.layer  = 1;
    h.lv     = cal.getFloat("lv", i);
    h.lw     = cal.getFloat("lw", i);
    h.lu     = cal.getFloat("lu", i);
    out_hits.push_back(h);
  }
}

bool RGAFiducialFilter::PassPCalEdgeCuts(const std::vector<CalHit>& hits) const
{
  float min_lv = std::numeric_limits<float>::infinity();
  float min_lw = std::numeric_limits<float>::infinity();
  for (const auto& h : hits) {
    if (h.lv < min_lv) min_lv = h.lv;
    if (h.lw < min_lw) min_lw = h.lw;
  }
  const float thr = (m_strictness==1 ?  9.0f : m_strictness==2 ? 13.5f : 18.0f);
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
    if (r < m_ft.rmin || r > m_ft.rmax) return false;
    for (auto const& H : m_ft.holes) {
      const double d = std::hypot(x - H[1], y - H[2]);
      if (d < H[0]) return false;
    }
    return true; // first FT match decides
  }
  return true; // no FT association
}

} // namespace iguana::clas12