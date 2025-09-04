// =============================
// RGAFiducialFilter (rewritten)
// =============================
#include <cstdlib>
#include <cstring>   // std::strncasecmp (POSIX); if not available, remove the "false" check below
#include <sstream>

// Small helper so we can turn env knobs on/off easily.
// Treat "", "0", "false" (case-insensitive) as OFF; anything else as ON.
static bool env_on(const char* name) {
  const char* v = std::getenv(name);
  if (!v) return false;
  if (v[0] == '\0') return false;
  if (v[0] == '0') return false;
#if defined(__APPLE__) || defined(__linux__) || defined(__unix__)
  if (!strncasecmp(v, "false", 5)) return false;
#endif
  return true;
}

void RGAFiducialFilter::Start(hipo::banklist& banks)
{
  // Load this algorithm's YAML (safe if missing; we handle defaults)
  ParseYAMLConfig();

  // Debug toggles (env-driven)
  const bool dbg_all    = env_on("IGUANA_RGAFID_DEBUG");
  const bool dbg_masks  = dbg_all || env_on("IGUANA_RGAFID_DEBUG_MASKS");
  const bool dbg_ft     = dbg_all || env_on("IGUANA_RGAFID_DEBUG_FT");
  const int  dbg_nevt   = [=]{
    if (const char* e = std::getenv("IGUANA_RGAFID_DEBUG_EVENTS")) {
      try { return std::max(0, std::stoi(e)); } catch (...) {}
    }
    return 0;
  }();

  if (dbg_all) {
    m_log->Info("[RGAFID][DEBUG] enabled. masks={}, ft={}, events={}",
                dbg_masks ? "true" : "false",
                dbg_ft    ? "true" : "false",
                dbg_nevt);
  }

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
      // NOTE: pass FULL path including leaf
      auto v = GetOptionVector<int>("cal.strictness",
                                    YAMLReader::node_path_t{ "calorimeter", "strictness" });
      if (!v.empty()) u_strictness_user = std::clamp(v.front(), 1, 3);
    } catch (...) { /* keep default */ }
  }
  if (!u_strictness_user.has_value()) u_strictness_user = 1;

  if (dbg_all) {
    m_log->Info("[RGAFID] GetConfig() {}", GetConfig() ? "present" : "ABSENT");
    m_log->Info("[RGAFID] strictness final = {}", *u_strictness_user);
  }

  // Forward Tagger parameters from YAML (optional; defaults if missing)
  u_ft_params = FTParams{}; // default rmin=8.5, rmax=15.5, empty holes

  // Try FT.radius
  try {
    auto r = GetOptionVector<double>("radius",
                                     YAMLReader::node_path_t{ "forward_tagger", "radius" });
    if (r.size() >= 2) {
      const float a = static_cast<float>(r[0]);
      const float b = static_cast<float>(r[1]);
      u_ft_params.rmin = std::min(a, b);
      u_ft_params.rmax = std::max(a, b);
    }
  } catch (...) {
    if (dbg_ft) m_log->Info("[RGAFID][FT] radius YAML read failed: config file parsing issue");
  }

  // Try FT.holes_flat
  try {
    auto flat = GetOptionVector<double>("holes_flat",
                                        YAMLReader::node_path_t{ "forward_tagger", "holes_flat" });
    for (size_t i = 0; i + 2 < flat.size(); i += 3) {
      u_ft_params.holes.push_back({
        static_cast<float>(flat[i]),
        static_cast<float>(flat[i+1]),
        static_cast<float>(flat[i+2])
      });
    }
  } catch (...) {
    if (dbg_ft) m_log->Info("[RGAFID][FT] holes_flat YAML read failed: config file parsing issue");
  }

  if (dbg_ft) {
    m_log->Info("[RGAFID][FT] params: rmin={:.3f} rmax={:.3f} holes={}",
                u_ft_params.rmin, u_ft_params.rmax, u_ft_params.holes.size());
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
  const int runnum = configBank.getInt("run", 0);
  auto key = PrepareEvent(runnum);

  // Debug: event header
  if (env_on("IGUANA_RGAFID_DEBUG")) {
    m_log->Info("[RGAFID] Run(): run={} have_calor={} have_ft={} strictness={}",
                runnum, m_have_calor ? "true" : "false",
                m_have_ft ? "true" : "false",
                GetCalStrictness(key));
  }

  // filter tracks in place
  size_t idx = 0;
  particleBank.getMutableRowList().filter([this, calBankPtr, ftBankPtr, key, &idx](auto /*bank*/, auto row) {
    const int track_index = static_cast<int>(row);
    const bool accept = Filter(track_index, calBankPtr, ftBankPtr, key);
    if (env_on("IGUANA_RGAFID_DEBUG")) {
      m_log->Info("[RGAFID][track={} key={}] -> {}", track_index, key, accept ? "ACCEPT" : "REJECT");
      // Optional: stop spamming after N events if IGUANA_RGAFID_DEBUG_EVENTS is set
      if (const char* e = std::getenv("IGUANA_RGAFID_DEBUG_EVENTS")) {
        try {
          int limit = std::stoi(e);
          if (limit > 0 && ++idx >= static_cast<size_t>(limit)) {
            // no-op; we still process the event fully, just don't print more
          }
        } catch (...) {}
      }
    }
    return accept ? 1 : 0;
  });
}

void RGAFiducialFilter::Stop() { /* nothing */ }

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

  o_runnum->Save(runnum, key);

  // calorimeter strictness (from user/env/YAML -> clamped)
  const int strict = std::clamp(u_strictness_user.value_or(1), 1, 3);
  o_cal_strictness->Save(strict, key);

  if (env_on("IGUANA_RGAFID_DEBUG"))
    m_log->Info("[RGAFID][Reload] run={} key={} strictness={}", runnum, key, strict);

  // build and cache masks per run (from YAML only)
  if (m_masks_by_run.find(runnum) == m_masks_by_run.end()) {
    m_masks_by_run.emplace(runnum, BuildCalMaskCache(runnum));
  }
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
      if (!PassCalStrictness(h, strictness)) {
        if (env_on("IGUANA_RGAFID_DEBUG")) {
          m_log->Info("[RGAFID][CAL] track={} strictness={} -> edge FAIL (lv1={}, lw1={})",
                      track_index, strictness, h.lv1, h.lw1);
        }
        return false;
      }
      if (strictness >= 2) {
        if (!PassCalDeadPMTMasks(h, key)) {
          if (env_on("IGUANA_RGAFID_DEBUG"))
            m_log->Info("[RGAFID][CAL] track={} -> dead-PMT mask FAIL", track_index);
          return false;
        }
      }
    }
  }

  // Forward Tagger: apply only if we have an FT bank
  if (!PassFTFiducial(track_index, ftBank)) {
    if (env_on("IGUANA_RGAFID_DEBUG"))
      m_log->Info("[RGAFID][FT] track={} -> FT FAIL", track_index);
    return false;
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

  if (!GetConfig()) {
    if (env_on("IGUANA_RGAFID_DEBUG_MASKS"))
      m_log->Info("[RGAFID][MASK] no config => masks disabled");
    return out; // no YAML -> no dead-PMT masks
  }

  // Figure out which entry under calorimeter.masks to use for this run.
  // We rely on YAMLReader::InRange("runs", run) to locate the right list item.
  // If anything fails, fall back to the "default" entry.
  auto make_path = [](int sector, const char* layer, const char* axis) {
    // Build the static tail for later append of the front ("calorimeter"/"masks"/index/default)
    return std::array<std::string,4>{ std::to_string(sector), layer, axis, "cal_mask" };
  };

  // For logging, attempt to capture which index we hit.
  int masks_index_for_log = -1;
  bool used_default = false;

  auto read_axis = [&](int sector, const char* layer, const char* axis) -> std::vector<window_t> {
    YAMLReader::node_path_t p;

    // Preferred: choose by run range if runnum>0
    if (runnum > 0) {
      try {
        // IMPORTANT: InRange() must be used *as a path segment*; the reader will select that list item.
        auto selector = GetConfig()->InRange("runs", runnum);
        if (masks_index_for_log < 0) {
          // Try to stringify the selector (for log only). If it throws, we leave it -1.
          try {
            std::stringstream ss; ss << selector;
            masks_index_for_log = std::stoi(ss.str());
          } catch (...) { masks_index_for_log = 0; }
          if (env_on("IGUANA_RGAFID_DEBUG_MASKS"))
            m_log->Info("[RGAFID][MASK] run={} -> masks index={}", runnum, masks_index_for_log);
        }

        p = { "calorimeter","masks", selector,
              "sectors", std::to_string(sector), layer, axis, "cal_mask" };
      } catch (...) {
        // Fallback to explicit "default" block if InRange failed or out-of-range
        used_default = true;
        p = { "calorimeter","masks","default",
              "sectors", std::to_string(sector), layer, axis, "cal_mask" };
      }
    } else {
      used_default = true;
      p = { "calorimeter","masks","default",
            "sectors", std::to_string(sector), layer, axis, "cal_mask" };
    }

    try {
      auto flat = GetOptionVector<double>("cal.masks", p);
      auto wins = to_windows_flat(flat);
      return wins;
    } catch (...) {
      if (env_on("IGUANA_RGAFID_DEBUG_MASKS"))
        m_log->Info("[RGAFID][MASK] cal_mask read EXC: config file parsing issue");
      return {};
    }
  };

  // Read all sectors/layers/axes
  size_t total_windows = 0;
  for (int s = 1; s <= 6; ++s) {
    SectorMasks sm;
    sm.pcal.lv  = read_axis(s, "pcal",  "lv");  total_windows += sm.pcal.lv.size();
    sm.pcal.lw  = read_axis(s, "pcal",  "lw");  total_windows += sm.pcal.lw.size();
    sm.pcal.lu  = read_axis(s, "pcal",  "lu");  total_windows += sm.pcal.lu.size();
    sm.ecin.lv  = read_axis(s, "ecin",  "lv");  total_windows += sm.ecin.lv.size();
    sm.ecin.lw  = read_axis(s, "ecin",  "lw");  total_windows += sm.ecin.lw.size();
    sm.ecin.lu  = read_axis(s, "ecin",  "lu");  total_windows += sm.ecin.lu.size();
    sm.ecout.lv = read_axis(s, "ecout", "lv");  total_windows += sm.ecout.lv.size();
    sm.ecout.lw = read_axis(s, "ecout", "lw");  total_windows += sm.ecout.lw.size();
    sm.ecout.lu = read_axis(s, "ecout", "lu");  total_windows += sm.ecout.lu.size();
    out.emplace(s, std::move(sm));
  }

  if (env_on("IGUANA_RGAFID_DEBUG_MASKS")) {
    m_log->Info("[RGAFID][MASK] run={} sectors={} total_windows={}",
                runnum, 6, total_windows);
    if (used_default)
      m_log->Info("[RGAFID][MASK] run={} used DEFAULT masks entry", runnum);
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

  const bool fail =
    in_any(h.lv1, sm.pcal.lv) || in_any(h.lw1, sm.pcal.lw) || in_any(h.lu1, sm.pcal.lu) ||
    in_any(h.lv4, sm.ecin.lv) || in_any(h.lw4, sm.ecin.lw) || in_any(h.lu4, sm.ecin.lu) ||
    in_any(h.lv7, sm.ecout.lv)|| in_any(h.lw7, sm.ecout.lw)|| in_any(h.lu7, sm.ecout.lu);

  if (env_on("IGUANA_RGAFID_DEBUG_MASKS")) {
    m_log->Info("[RGAFID][MASK] sector={} PCAL(lv={},lw={},lu={}) ECIN(lv={},lw={},lu={}) ECOUT(lv={},lw={},lu={}) -> {}",
                h.sector,
                h.lv1, h.lw1, h.lu1,
                h.lv4, h.lw4, h.lu4,
                h.lv7, h.lw7, h.lu7,
                fail ? "HIT-MASK" : "clear");
  }

  return !fail;
}

bool RGAFiducialFilter::PassFTFiducial(int track_index, const hipo::bank* ftBank) const
{
  // If the FT bank is not present for this file/event, we skip FT cuts (pass-through).
  if (ftBank == nullptr) return true;

  const bool dbg_ft = env_on("IGUANA_RGAFID_DEBUG") || env_on("IGUANA_RGAFID_DEBUG_FT");

  const int nrows = ftBank->getRows();
  for (int i = 0; i < nrows; ++i) {
    if (ftBank->getInt("pindex", i) != track_index) continue;

    const double x = ftBank->getFloat("x", i);
    const double y = ftBank->getFloat("y", i);
    const double r = std::sqrt(x*x + y*y);

    if (dbg_ft) {
      m_log->Info("[RGAFID][FT] track={} x={:.2f} y={:.2f} r={:.2f} rwin=[{:.2f},{:.2f}]",
                  track_index, x, y, r, u_ft_params.rmin, u_ft_params.rmax);
    }

    // radial window
    if (r < u_ft_params.rmin) return false;
    if (r > u_ft_params.rmax) return false;

    // holes (circles to exclude)
    for (auto const& h : u_ft_params.holes) {
      const double d = std::sqrt((x - h[1])*(x - h[1]) + (y - h[2])*(y - h[2]));
      if (d < h[0]) return false;
    }

    // this FT association passes
    return true;
  }

  // No FT association for this track in this event -> pass-through
  return true;
}

// -------------------------
// accessors (unchanged)
// -------------------------

int RGAFiducialFilter::GetRunNum(concurrent_key_t key) const { return o_runnum->Load(key); }
int RGAFiducialFilter::GetCalStrictness(concurrent_key_t key) const { return o_cal_strictness->Load(key); }