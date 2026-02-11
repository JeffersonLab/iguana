#include "Validator.h"

#include <cmath>
#include <iomanip>
#include <iostream>
#include <vector>

namespace iguana::clas12::rga {

  REGISTER_IGUANA_VALIDATOR(ProtonEnergyLossCorrectionValidator);

  // Compute theta (deg) from momentum components.
  double ProtonEnergyLossCorrectionValidator::ThetaDegFromPxPyPz(double px, double py, double pz) {
    double pt = std::sqrt(px * px + py * py);
    double th = std::atan2(pt, pz);
    return th * (180.0 / M_PI);
  }

  // Convert theta (deg) to a bin index.
  //
  // Returns:
  //   -1 if theta is outside [kThetaMinDeg, kThetaMaxDeg]
  //   otherwise an index in [0, kNBins-1].
  int ProtonEnergyLossCorrectionValidator::ThetaBinIndex(double theta_deg) {
    if(theta_deg < kThetaMinDeg) {
      return -1;
    }
    if(theta_deg > kThetaMaxDeg) {
      return -1;
    }
    if(theta_deg >= kThetaMaxDeg) {
      return kNBins - 1;
    }

    int idx = (int)((theta_deg - kThetaMinDeg) / kThetaStepDeg);
    if(idx < 0 || idx >= kNBins) {
      return -1;
    }
    return idx;
  }

  void ProtonEnergyLossCorrectionValidator::StartHook(hipo::banklist& banks) {
    // Cache bank indices 
    m_b_particle = GetBankIndex(banks, "REC::Particle");
    m_b_config   = GetBankIndex(banks, "RUN::config");

    // Build and start an AlgorithmSequence
    m_algo_seq = std::make_unique<AlgorithmSequence>();
    m_algo_seq->Add("clas12::rga::ProtonEnergyLossCorrection");
    m_algo_seq->Start(banks);

    // Reset counters/accumulators.
    for(auto& b : m_bins) {
      b.n = 0;
      b.sum_p_before = 0.0;
      b.sum_p_after  = 0.0;
    }
    m_total_protons_in_range = 0;
    m_total_protons_all      = 0;
  }

  bool ProtonEnergyLossCorrectionValidator::RunHook(hipo::banklist& banks) const {
    auto& particle = GetBank(banks, m_b_particle, "REC::Particle");
    auto& config   = GetBank(banks, m_b_config, "RUN::config");
    (void)config; // run number is not needed for summary of \Delta p.

    // record before values, run the algorithm, compute after values.
    // store the particle row index and p_before to we can match the same
    // rows after the algorithm modifies the bank in place.
    struct Entry {
      int row = -1;
      int bin = -1;
      double p_before = 0.0;
    };

    std::vector<Entry> entries;
    entries.reserve((size_t)particle.getRows());

    int const nrows = particle.getRows();

    // snapshot before for all protons.
    for(int i = 0; i < nrows; ++i) {
      int pid = particle.getInt("pid", i);
      if(pid != 2212) {
        continue;
      }

      double px = (double)particle.getFloat("px", i);
      double py = (double)particle.getFloat("py", i);
      double pz = (double)particle.getFloat("pz", i);

      double p = std::sqrt(px * px + py * py + pz * pz);
      double theta_deg = ThetaDegFromPxPyPz(px, py, pz);
      int bin = ThetaBinIndex(theta_deg);

      Entry e;
      e.row = i;
      e.bin = bin;
      e.p_before = p;
      entries.push_back(e);
    }

    // Run algorithm under test 
    m_algo_seq->Run(banks);

    // Accumulate after values.
    std::scoped_lock<std::mutex> lock(m_mutex);

    for(auto const& e : entries) {
      m_total_protons_all++;

      if(e.bin < 0) {
        continue;
      }
      m_total_protons_in_range++;

      int i = e.row;

      double px = (double)particle.getFloat("px", i);
      double py = (double)particle.getFloat("py", i);
      double pz = (double)particle.getFloat("pz", i);

      double p_after = std::sqrt(px * px + py * py + pz * pz);

      auto& B = m_bins[(size_t)e.bin];
      B.n++;
      B.sum_p_before += e.p_before;
      B.sum_p_after  += p_after;
    }

    return true;
  }

  void ProtonEnergyLossCorrectionValidator::StopHook() {
    // human-readable summary table
    std::cout << "\n";
    std::cout << "ProtonEnergyLossCorrectionValidator summary\n";
    std::cout << "  theta bins: " << kThetaMinDeg << " to " << kThetaMaxDeg
              << " (deg) in steps of " << kThetaStepDeg << " (deg)\n";
    std::cout << "  total protons (all theta): " << m_total_protons_all << "\n";
    std::cout << "  total protons in theta range: " << m_total_protons_in_range << "\n\n";

    std::cout << std::fixed << std::setprecision(6);
    std::cout << "  BinRangeTheta(deg)    N        <p_before>(GeV)    <p_after>(GeV)    <delta>(GeV)\n";
    std::cout << "  -------------------------------------------------------------------------------------\n";

    for(int ib = 0; ib < kNBins; ++ib) {
      double lo = kThetaMinDeg + (double)ib * kThetaStepDeg;
      double hi = lo + kThetaStepDeg;

      auto const& B = m_bins[(size_t)ib];

      if(B.n <= 0) {
        std::cout << "  [" << std::setw(4) << lo << "," << std::setw(4) << hi << ")"
                  << "        " << std::setw(8) << 0
                  << "        " << std::setw(14) << 0.0
                  << "        " << std::setw(13) << 0.0
                  << "        " << std::setw(12) << 0.0
                  << "\n";
        continue;
      }

      double mean_before = B.sum_p_before / (double)B.n;
      double mean_after  = B.sum_p_after  / (double)B.n;
      double delta       = mean_after - mean_before;

      std::cout << "  [" << std::setw(4) << lo << "," << std::setw(4) << hi << ")"
                << "        " << std::setw(8) << B.n
                << "        " << std::setw(14) << mean_before
                << "        " << std::setw(13) << mean_after
                << "        " << std::setw(12) << delta
                << "\n";
    }

    std::cout << "\n";
  }

} 