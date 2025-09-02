#pragma once

#include "iguana/algorithms/Algorithm.h"
#include "iguana/algorithms/TypeDefs.h"
#include "iguana/services/ConcurrentParam.h"

#include <unordered_map>
#include <mutex>

namespace iguana::clas12 {

  /// @brief_algo RGA fiducial filter with (for now) calorimeter-only cuts and strictness.
  /// To do: add FT, DC, CVT fiducial cuts
  ///
  /// @begin_doc_algo{clas12::RGAFiducialFilter | Filter}
  /// @input_banks{REC::Particle (tracks), REC::Calorimeter, RUN::config}
  /// @output_banks{REC::Particle (tracks)}
  /// @end_doc
  ///
  /// @begin_doc_config{clas12/RGAFiducialFilter}
  /// @config_param{calorimeter/strictness | int | 1,2,3: tighter = larger PCAL edge margins; >=2 also enables dead-PMT masks}
  /// @end_doc
  class RGAFiducialFilter : public Algorithm
  {
      DEFINE_IGUANA_ALGORITHM(RGAFiducialFilter, clas12::RGAFiducialFilter)

    public:

      // --- helpers and data structures for calorimeter linking ---
      struct CalLayers {
        int   sector = 0;
        float lv1=0.f, lw1=0.f, lu1=0.f; // layer 1 (PCAL)
        float lv4=0.f, lw4=0.f, lu4=0.f; // layer 4 (ECin)
        float lv7=0.f, lw7=0.f, lu7=0.f; // layer 7 (ECout)
        bool  has_any = false;           // saw at least one matching cal row
      };
      
      void Start(hipo::banklist& banks) override;
      void Run  (hipo::banklist& banks) const override;
      void Stop () override;

      /// @action_function{reload} prepare the event and cache per-run options
      /// @when_to_call{for each event}
      /// @param runnum the run number
      /// @returns a key to pass to Filter
      concurrent_key_t PrepareEvent(int runnum) const;

      /// Core filter (calorimeter for now). Applies to any track with associated calorimeter hits.
      /// Tracks without a calorimeter association are passed through unchanged.
      /// @when_to_call{for each track}
      /// @param track_index row index in REC::Particle (used to link to REC::Calorimeter via pindex)
      /// @param calBank     REC::Calorimeter bank for the current event
      /// @param key         return value of PrepareEvent
      /// @returns true if the track passes fiducial cuts (or has no calorimeter hit)
      bool Filter(int track_index,
                  const hipo::bank& calBank,
                  concurrent_key_t key) const;

      // Accessors (thread-safe via ConcurrentParam)
      int GetRunNum(concurrent_key_t key) const;
      int GetCalStrictness(concurrent_key_t key) const;

    private:

      /// Collect PCAL/ECin/ECout (layers 1/4/7) for a given track index
      static CalLayers CollectCalHitsForTrack(const hipo::bank& calBank, int track_index);

      /// Strictness-only PCAL edge cuts (mirrors the Java thresholds)
      static bool PassCalStrictness(const CalLayers& h, int strictness);

      /// Dead-PMT masks (run and sector dependent; only applied for strictness >= 2)
      bool PassCalDeadPMTMasks(const CalLayers& h, int runnum) const;

      /// Load per-run options from YAML (calorimeter.strictness)
      void Reload(int runnum, concurrent_key_t key) const;

      // bank indices
      hipo::banklist::size_type b_particle{}, b_calor{}, b_config{};

      // cached (per-run) config
      mutable std::unique_ptr<ConcurrentParam<int>> o_runnum;
      mutable std::unique_ptr<ConcurrentParam<int>> o_cal_strictness;

      mutable std::mutex m_mutex;
  };

} // namespace iguana::clas12