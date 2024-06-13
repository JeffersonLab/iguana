#include "Algorithm.h"
#include <cmath>

namespace iguana::clas12 {

  REGISTER_IGUANA_ALGORITHM(MomentumCorrection);

  void MomentumCorrection::Start(hipo::banklist& banks)
  {
    b_particle = GetBankIndex(banks, "REC::Particle");
    b_sector   = GetBankIndex(banks, "REC::Particle::Sector");
    b_config   = GetBankIndex(banks, "RUN::config");
  }


  void MomentumCorrection::Run(hipo::banklist& banks) const
  {
    auto& particleBank = GetBank(banks, b_particle, "REC::Particle");
    auto& sectorBank   = GetBank(banks, b_sector, "REC::Particle::Sector");
    auto& configBank   = GetBank(banks, b_config, "RUN::config");
    ShowBank(particleBank, Logger::Header("INPUT PARTICLES"));

    auto torus   = configBank.getFloat("torus", 0);

    for(auto const& row : particleBank.getRowList()) {

      auto [px, py, pz] = Transform(
          particleBank.getFloat("px", row),
          particleBank.getFloat("py", row),
          particleBank.getFloat("pz", row),
          sectorBank.getInt("sector", row),
          particleBank.getInt("pid", row),
          torus);
      particleBank.putFloat("px", row, px);
      particleBank.putFloat("py", row, py);
      particleBank.putFloat("pz", row, pz);
    }

    ShowBank(particleBank, Logger::Header("OUTPUT PARTICLES"));
  }


  vector3_t MomentumCorrection::Transform(vector_element_t px, vector_element_t py, vector_element_t pz, int sec, int pid, float torus) const
  {
    // energy loss correction
    auto e_cor = torus < 0
                     ? EnergyLossInbending(px, py, pz, pid)
                     : EnergyLossOutbending(px, py, pz, pid);
    // momentum correction
    auto p_cor = torus < 0
                     ? CorrectionInbending(e_cor * px, e_cor * py, e_cor * pz, sec, pid)
                     : CorrectionOutbending(e_cor * px, e_cor * py, e_cor * pz, sec, pid);
    // return the corrected momentum
    return {
        e_cor * p_cor * px,
        e_cor * p_cor * py,
        e_cor * p_cor * pz};
  }


  double MomentumCorrection::CorrectionInbending(vector_element_t const Px, vector_element_t const Py, vector_element_t const Pz, int const sec, int const pid) const
  {

    // skip the correction if it's not defined
    if(!(pid == particle::electron || pid == particle::pi_plus || pid == particle::pi_minus || pid == particle::proton))
      return 1.0;

    // Momentum Magnitude
    double pp = sqrt(Px * Px + Py * Py + Pz * Pz);

    // Initializing the correction factor
    double dp = 0;

    // Defining Phi Angle
    double Phi = (180 / M_PI) * atan2(Py, Px);

    // (Initial) Shift of the Phi Angle (done to realign sectors whose data is separated when plotted from ±180˚)
    if(((sec == 4 || sec == 3) && Phi < 0) || (sec > 4 && Phi < 90)) {
      Phi += 360;
    }

    // Getting Local Phi Angle
    double PhiLocal = Phi - (sec - 1) * 60;

    // Applying Shift Functions to Phi Angles (local shifted phi = phi)
    double phi = PhiLocal;

    // For Electron Shift
    if(pid == particle::electron) {
      phi = PhiLocal - 30 / pp;
    }

    // For π+ Pion/Proton Shift
    if(pid == particle::pi_plus || pid == particle::proton) {
      phi = PhiLocal + (32 / (pp - 0.05));
    }

    // For π- Pion Shift
    if(pid == particle::pi_minus) {
      phi = PhiLocal - (32 / (pp - 0.05));
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //==================================================================================================================================//
    //=======================//=======================//     Electron Corrections     //=======================//=======================//
    //==================================================================================================================================//
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    if(pid == particle::electron) {
      if(sec == 1) {
        // The CONTINUOUS QUADRATIC function predicted for ∆p_{El} for [Cor = Uncorrected][Sector 1] is:
        dp = ((-4.3303e-06) * phi * phi + (1.1006e-04) * phi + (-5.7235e-04)) * pp * pp + ((3.2555e-05) * phi * phi + (-0.0014559) * phi + (0.0014878)) * pp + ((-1.9577e-05) * phi * phi + (0.0017996) * phi + (0.025963));
      }
      if(sec == 2) {
        // The CONTINUOUS QUADRATIC function predicted for ∆p_{El} for [Cor = Uncorrected][Sector 2] is:
        dp = ((-9.8045e-07) * phi * phi + (6.7395e-05) * phi + (-4.6757e-05)) * pp * pp + ((-1.4958e-05) * phi * phi + (-0.0011191) * phi + (-0.0025143)) * pp + ((1.2699e-04) * phi * phi + (0.0033121) * phi + (0.020819));
      }
      if(sec == 3) {
        // The CONTINUOUS QUADRATIC function predicted for ∆p_{El} for [Cor = Uncorrected][Sector 3] is:
        dp = ((-5.9459e-07) * phi * phi + (-2.8289e-05) * phi + (-4.3541e-04)) * pp * pp + ((-1.5025e-05) * phi * phi + (5.7730e-04) * phi + (-0.0077582)) * pp + ((7.3348e-05) * phi * phi + (-0.001102) * phi + (0.057052));
      }
      if(sec == 4) {
        // The CONTINUOUS QUADRATIC function predicted for ∆p_{El} for [Cor = Uncorrected][Sector 4] is:
        dp = ((-2.2714e-06) * phi * phi + (-3.0360e-05) * phi + (-8.9322e-04)) * pp * pp + ((2.9737e-05) * phi * phi + (5.1142e-04) * phi + (0.0045641)) * pp + ((-1.0582e-04) * phi * phi + (-5.6852e-04) * phi + (0.027506));
      }
      if(sec == 5) {
        // The CONTINUOUS QUADRATIC function predicted for ∆p_{El} for [Cor = Uncorrected][Sector 5] is:
        dp = ((-1.1490e-06) * phi * phi + (-6.2147e-06) * phi + (-4.7235e-04)) * pp * pp + ((3.7039e-06) * phi * phi + (-1.5943e-04) * phi + (-8.5238e-04)) * pp + ((4.4069e-05) * phi * phi + (0.0014152) * phi + (0.031933));
      }
      if(sec == 6) {
        // The CONTINUOUS QUADRATIC function predicted for ∆p_{El} for [Cor = Uncorrected][Sector 6] is:
        dp = ((1.1076e-06) * phi * phi + (4.0156e-05) * phi + (-1.6341e-04)) * pp * pp + ((-2.8613e-05) * phi * phi + (-5.1861e-04) * phi + (-0.0056437)) * pp + ((1.2419e-04) * phi * phi + (4.9084e-04) * phi + (0.049976));
      }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //====================================================================================================================================//
    //=========================//=========================//     π+ Corrections     //=========================//=========================//
    //====================================================================================================================================//
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    if(pid == particle::pi_plus) {
      if(sec == 1) {
        dp = ((-5.4904e-07) * phi * phi + (-1.4436e-05) * phi + (3.1534e-04)) * pp * pp + ((3.8231e-06) * phi * phi + (3.6582e-04) * phi + (-0.0046759)) * pp + ((-5.4913e-06) * phi * phi + (-4.0157e-04) * phi + (0.010767));
        dp = dp + ((6.1103e-07) * phi * phi + (5.5291e-06) * phi + (-1.9120e-04)) * pp * pp + ((-3.2300e-06) * phi * phi + (1.5377e-05) * phi + (7.5279e-04)) * pp + ((2.1434e-06) * phi * phi + (-6.9572e-06) * phi + (-7.9333e-05));
        dp = dp + ((-1.3049e-06) * phi * phi + (1.1295e-05) * phi + (4.5797e-04)) * pp * pp + ((9.3122e-06) * phi * phi + (-5.1074e-05) * phi + (-0.0030757)) * pp + ((-1.3102e-05) * phi * phi + (2.2153e-05) * phi + (0.0040938));
      }
      if(sec == 2) {
        dp = ((-1.0087e-06) * phi * phi + (2.1319e-05) * phi + (7.8641e-04)) * pp * pp + ((6.7485e-06) * phi * phi + (7.3716e-05) * phi + (-0.0094591)) * pp + ((-1.1820e-05) * phi * phi + (-3.8103e-04) * phi + (0.018936));
        dp = dp + ((8.8155e-07) * phi * phi + (-2.8257e-06) * phi + (-2.6729e-04)) * pp * pp + ((-5.4499e-06) * phi * phi + (3.8397e-05) * phi + (0.0015914)) * pp + ((6.8926e-06) * phi * phi + (-5.9386e-05) * phi + (-0.0021749));
        dp = dp + ((-2.0147e-07) * phi * phi + (1.1061e-05) * phi + (3.8827e-04)) * pp * pp + ((4.9294e-07) * phi * phi + (-6.0257e-05) * phi + (-0.0022087)) * pp + ((9.8548e-07) * phi * phi + (5.9047e-05) * phi + (0.0022905));
      }
      if(sec == 3) {
        dp = ((8.6722e-08) * phi * phi + (-1.7975e-05) * phi + (4.8118e-05)) * pp * pp + ((2.6273e-06) * phi * phi + (3.1453e-05) * phi + (-0.0015943)) * pp + ((-6.4463e-06) * phi * phi + (-5.8990e-05) * phi + (0.0041703));
        dp = dp + ((9.6317e-07) * phi * phi + (-1.7659e-06) * phi + (-8.8318e-05)) * pp * pp + ((-5.1346e-06) * phi * phi + (8.3318e-06) * phi + (3.7723e-04)) * pp + ((3.9548e-06) * phi * phi + (-6.9614e-05) * phi + (2.1393e-04));
        dp = dp + ((5.6438e-07) * phi * phi + (8.1678e-06) * phi + (-9.4406e-05)) * pp * pp + ((-3.9074e-06) * phi * phi + (-6.5174e-05) * phi + (5.4218e-04)) * pp + ((6.3198e-06) * phi * phi + (1.0611e-04) * phi + (-4.5749e-04));
      }
      if(sec == 4) {
        dp = ((4.3406e-07) * phi * phi + (-4.9036e-06) * phi + (2.3064e-04)) * pp * pp + ((1.3624e-06) * phi * phi + (3.2907e-05) * phi + (-0.0034872)) * pp + ((-5.1017e-06) * phi * phi + (2.4593e-05) * phi + (0.0092479));
        dp = dp + ((6.0218e-07) * phi * phi + (-1.4383e-05) * phi + (-3.1999e-05)) * pp * pp + ((-1.1243e-06) * phi * phi + (9.3884e-05) * phi + (-4.1985e-04)) * pp + ((-1.8808e-06) * phi * phi + (-1.2222e-04) * phi + (0.0014037));
        dp = dp + ((-2.5490e-07) * phi * phi + (-8.5120e-07) * phi + (7.9109e-05)) * pp * pp + ((2.5879e-06) * phi * phi + (8.6108e-06) * phi + (-5.1533e-04)) * pp + ((-4.4521e-06) * phi * phi + (-1.7012e-05) * phi + (7.4848e-04));
      }
      if(sec == 5) {
        dp = ((2.4292e-07) * phi * phi + (8.8741e-06) * phi + (2.9482e-04)) * pp * pp + ((3.7229e-06) * phi * phi + (7.3215e-06) * phi + (-0.0050685)) * pp + ((-1.1974e-05) * phi * phi + (-1.3043e-04) * phi + (0.0078836));
        dp = dp + ((1.0867e-06) * phi * phi + (-7.7630e-07) * phi + (-4.4930e-05)) * pp * pp + ((-5.6564e-06) * phi * phi + (-1.3417e-05) * phi + (2.5224e-04)) * pp + ((6.8460e-06) * phi * phi + (9.0495e-05) * phi + (-4.6587e-04));
        dp = dp + ((8.5720e-07) * phi * phi + (-6.7464e-06) * phi + (-4.0944e-05)) * pp * pp + ((-4.7370e-06) * phi * phi + (5.8808e-05) * phi + (1.9047e-04)) * pp + ((5.7404e-06) * phi * phi + (-1.1105e-04) * phi + (-1.9392e-04));
      }
      if(sec == 6) {
        dp = ((2.1191e-06) * phi * phi + (-3.3710e-05) * phi + (2.5741e-04)) * pp * pp + ((-1.2915e-05) * phi * phi + (2.3753e-04) * phi + (-2.6882e-04)) * pp + ((2.2676e-05) * phi * phi + (-2.3115e-04) * phi + (-0.001283));
        dp = dp + ((6.0270e-07) * phi * phi + (-6.8200e-06) * phi + (1.3103e-04)) * pp * pp + ((-1.8745e-06) * phi * phi + (3.8646e-05) * phi + (-8.8056e-04)) * pp + ((2.0885e-06) * phi * phi + (-3.4932e-05) * phi + (4.5895e-04));
        dp = dp + ((4.7349e-08) * phi * phi + (-5.7528e-06) * phi + (-3.4097e-06)) * pp * pp + ((1.7731e-06) * phi * phi + (3.5865e-05) * phi + (-5.7881e-04)) * pp + ((-9.7008e-06) * phi * phi + (-4.1836e-05) * phi + (0.0035403));
      }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //====================================================================================================================================//
    //==================//==================//    π- Corrections (Updated as of 01-13-2023)    //==================//==================//
    //====================================================================================================================================//
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    if(pid == particle::pi_minus) {
      if(sec == 1) {

        dp = (-9.2163E-07 * phi * phi + 3.1862E-06 * phi + 2.9805E-03) * pp * pp + (1.0435E-05 * phi * phi + -8.7298E-05 * phi + -1.7730E-02) * pp + -1.5154E-05 * phi * phi + -1.3716E-04 * phi + 2.2410E-02;
      }
      if(sec == 2) {

        dp = (-1.9656E-06 * phi * phi + 9.7389E-05 * phi + 4.1250E-03) * pp * pp + (1.6439E-06 * phi * phi + -4.6007E-04 * phi + -1.9809E-02) * pp + 3.5794E-07 * phi * phi + 4.8250E-04 * phi + 1.7333E-02;
      }

      if(sec == 3) {

        dp = (2.5351E-06 * phi * phi + 4.1043E-05 * phi + 3.1157E-03) * pp * pp + (-1.3573E-05 * phi * phi + -1.7609E-04 * phi + -1.6759E-02) * pp + 1.4647E-05 * phi * phi + 1.7484E-04 * phi + 1.3805E-02;
      }

      if(sec == 4) {

        dp = (2.3500E-06 * phi * phi + -7.7894E-05 * phi + 4.4837E-03) * pp * pp + (-9.7915E-06 * phi * phi + 4.6576E-04 * phi + -2.6809E-02) * pp + 1.3819E-05 * phi * phi + -5.6017E-04 * phi + 3.0320E-02;
      }

      if(sec == 5) {

        dp = (-2.1809E-06 * phi * phi + 2.4948E-05 * phi + 2.7995E-03) * pp * pp + (6.3908E-06 * phi * phi + -6.5122E-05 * phi + -1.7571E-02) * pp + -1.9146E-06 * phi * phi + -6.3799E-05 * phi + 2.0877E-02;
      }

      if(sec == 6) {

        dp = (-9.3043E-06 * phi * phi + 6.2678E-05 * phi + 5.9660E-03) * pp * pp + (4.0581E-05 * phi * phi + -3.0537E-04 * phi + -3.1485E-02) * pp + -3.8345E-05 * phi * phi + 2.0267E-04 * phi + 3.3363E-02;
      }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //====================================================================================================================================//
    //=======================//=======================//     All Proton Corrections     //=======================//=======================//
    //====================================================================================================================================//
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    if(pid == particle::proton) {
      if(sec == 1) {
        dp = ((1 + std::copysign(1, (pp - 1.4))) / 2) * ((4.4034e-03) * pp + (-0.01703)) + ((1 + std::copysign(1, -(pp - 1.4))) / 2) * ((-0.10898) * (pp - 1.4) * (pp - 1.4) + (-0.09574) * (pp - 1.4) + ((4.4034e-03) * 1.4 + (-0.01703)));
      }
      if(sec == 2) {
        dp = ((1 + std::copysign(1, (pp - 1.5))) / 2) * ((0.01318) * pp + (-0.03403)) + ((1 + std::copysign(1, -(pp - 1.5))) / 2) * ((-0.09829) * (pp - 1.5) * (pp - 1.5) + (-0.0986) * (pp - 1.5) + ((0.01318) * 1.5 + (-0.03403)));
      }
      if(sec == 3) {
        dp = ((1 + std::copysign(1, (pp - 1.05))) / 2) * ((-4.7052e-03) * pp + (1.2410e-03)) + ((1 + std::copysign(1, -(pp - 1.05))) / 2) * ((-0.22721) * (pp - 1.05) * (pp - 1.05) + (-0.09702) * (pp - 1.05) + ((-4.7052e-03) * 1.05 + (1.2410e-03)));
      }
      if(sec == 4) {
        dp = ((1 + std::copysign(1, (pp - 1.4))) / 2) * ((-1.0900e-03) * pp + (-4.0573e-03)) + ((1 + std::copysign(1, -(pp - 1.4))) / 2) * ((-0.09236) * (pp - 1.4) * (pp - 1.4) + (-0.073) * (pp - 1.4) + ((-1.0900e-03) * 1.4 + (-4.0573e-03)));
      }
      if(sec == 5) {
        dp = ((1 + std::copysign(1, (pp - 1.5))) / 2) * ((7.3965e-03) * pp + (-0.02428)) + ((1 + std::copysign(1, -(pp - 1.5))) / 2) * ((-0.09539) * (pp - 1.5) * (pp - 1.5) + (-0.09263) * (pp - 1.5) + ((7.3965e-03) * 1.5 + (-0.02428)));
      }
      if(sec == 6) {
        dp = ((1 + std::copysign(1, (pp - 1.15))) / 2) * ((-7.6214e-03) * pp + (8.1014e-03)) + ((1 + std::copysign(1, -(pp - 1.15))) / 2) * ((-0.12718) * (pp - 1.15) * (pp - 1.15) + (-0.06626) * (pp - 1.15) + ((-7.6214e-03) * 1.15 + (8.1014e-03)));
      }
    }

    return dp / pp + 1;
  }


  double MomentumCorrection::CorrectionOutbending(vector_element_t const Px, vector_element_t const Py, vector_element_t const Pz, int const sec, int const pid) const
  {

    // skip the correction if it's not defined
    if(!(pid == particle::electron || pid == particle::pi_plus || pid == particle::pi_minus))
      return 1.0;

    // Momentum Magnitude
    double pp = sqrt(Px * Px + Py * Py + Pz * Pz);

    // Initializing the correction factor
    double dp = 0;

    // Defining Phi Angle
    double Phi = (180 / M_PI) * atan2(Py, Px);

    // (Initial) Shift of the Phi Angle (done to realign sectors whose data is separated when plotted from ±180˚)
    if(((sec == 4 || sec == 3) && Phi < 0) || (sec > 4 && Phi < 90)) {
      Phi += 360;
    }

    // Getting Local Phi Angle
    double PhiLocal = Phi - (sec - 1) * 60;

    // Applying Shift Functions to Phi Angles (local shifted phi = phi)
    double phi = PhiLocal;
    // For Electron Shift
    if(pid == particle::electron) {
      phi = PhiLocal - 30 / pp;
    }
    // For π+ Pion/Proton Shift
    if(pid == particle::pi_plus || pid == particle::proton) {
      phi = PhiLocal + (32 / (pp - 0.05));
    }
    // For π- Pion Shift
    if(pid == particle::pi_minus) {
      phi = PhiLocal - (32 / (pp - 0.05));
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //=======================//=======================//     Electron Corrections     //=======================//=======================//
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    if(pid == particle::electron) {
      if(sec == 1) {
        dp = ((1.3189e-06) * phi * phi + (4.26057e-05) * phi + (-0.002322628)) * pp * pp + ((-1.1409e-05) * phi * phi + (2.2188e-05) * phi + (0.02878927)) * pp + ((2.4950e-05) * phi * phi + (1.6170e-06) * phi + (-0.061816275));
      }
      if(sec == 2) {
        dp = ((-2.9240e-07) * phi * phi + (3.2448e-07) * phi + (-0.001848308)) * pp * pp + ((4.4500e-07) * phi * phi + (4.76324e-04) * phi + (0.02219469)) * pp + ((6.9220e-06) * phi * phi + (-0.00153517) * phi + (-0.0479058));
      }
      if(sec == 3) {
        dp = ((2.71911e-06) * phi * phi + (1.657148e-05) * phi + (-0.001822211)) * pp * pp + ((-4.96814e-05) * phi * phi + (-3.761117e-04) * phi + (0.02564148)) * pp + ((1.97748e-04) * phi * phi + (9.58259e-04) * phi + (-0.05818292));
      }
      if(sec == 4) {
        dp = ((1.90966e-06) * phi * phi + (-2.4761e-05) * phi + (-0.00231562)) * pp * pp + ((-2.3927e-05) * phi * phi + (2.25262e-04) * phi + (0.0291831)) * pp + ((8.0515e-05) * phi * phi + (-6.42098e-04) * phi + (-0.06159197));
      }
      if(sec == 5) {
        dp = ((-3.6760323e-06) * phi * phi + (4.04398e-05) * phi + (-0.0021967515)) * pp * pp + ((4.90857e-05) * phi * phi + (-4.37437e-04) * phi + (0.02494339)) * pp + ((-1.08257e-04) * phi * phi + (0.00146111) * phi + (-0.0648485));
      }
      if(sec == 6) {
        dp = ((-6.2488e-08) * phi * phi + (2.23173e-05) * phi + (-0.00227522)) * pp * pp + ((1.8372e-05) * phi * phi + (-7.5227e-05) * phi + (0.032636)) * pp + ((-6.6566e-05) * phi * phi + (-2.4450e-04) * phi + (-0.072293));
      }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //=========================//=========================//     π+ Corrections     //=========================//=========================//
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    if(pid == particle::pi_plus) {
      if(sec == 1) {
        dp = ((-1.7334e-06) * phi * phi + (1.45112e-05) * phi + (0.00150721)) * pp * pp + ((6.6234e-06) * phi * phi + (-4.81191e-04) * phi + (-0.0138695)) * pp + ((-3.23625e-06) * phi * phi + (2.79751e-04) * phi + (0.027726));
      }
      if(sec == 2) {
        dp = ((-4.475464e-06) * phi * phi + (-4.11573e-05) * phi + (0.00204557)) * pp * pp + ((2.468278e-05) * phi * phi + (9.3590e-05) * phi + (-0.015399)) * pp + ((-1.61547e-05) * phi * phi + (-2.4206e-04) * phi + (0.0231743));
      }
      if(sec == 3) {
        dp = ((-8.0374e-07) * phi * phi + (2.8728e-06) * phi + (0.00152163)) * pp * pp + ((5.1347e-06) * phi * phi + (3.71709e-04) * phi + (-0.0165735)) * pp + ((4.0105e-06) * phi * phi + (-5.289869e-04) * phi + (0.02175395));
      }
      if(sec == 4) {
        dp = ((-3.8790e-07) * phi * phi + (-4.78445e-05) * phi + (0.002324725)) * pp * pp + ((6.80543e-06) * phi * phi + (5.69358e-04) * phi + (-0.0199162)) * pp + ((-1.30264e-05) * phi * phi + (-5.91606e-04) * phi + (0.03202088));
      }
      if(sec == 5) {
        dp = ((2.198518e-06) * phi * phi + (-1.52535e-05) * phi + (0.001187761)) * pp * pp + ((-1.000264e-05) * phi * phi + (1.63976e-04) * phi + (-0.01429673)) * pp + ((9.4962e-06) * phi * phi + (-3.86691e-04) * phi + (0.0303695));
      }
      if(sec == 6) {
        dp = ((-3.92944e-07) * phi * phi + (1.45848e-05) * phi + (0.00120668)) * pp * pp + ((3.7899e-06) * phi * phi + (-1.98219e-04) * phi + (-0.0131312)) * pp + ((-3.9961e-06) * phi * phi + (-1.32883e-04) * phi + (0.0294497));
      }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //=======================//=======================//      π- Corrections            //=======================//=======================//
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    if(pid == particle::pi_minus) {
      if(sec == 1) {
        dp = (7.8044E-06 * phi * phi + -9.4703E-05 * phi + 4.6696E-03) * pp * pp + (-3.4668E-05 * phi * phi + 6.2280E-04 * phi + -2.4273E-02) * pp + 2.3566E-05 * phi * phi + -5.8519E-04 * phi + 3.9226E-02;
      }
      if(sec == 2) {
        dp = (-4.6611E-06 * phi * phi + -8.1637E-05 * phi + 7.5013E-03) * pp * pp + (1.7616E-05 * phi * phi + 3.5439E-04 * phi + -3.7122E-02) * pp + -1.6286E-05 * phi * phi + -2.6545E-04 * phi + 4.5659E-02;
      }
      if(sec == 3) {
        dp = (4.5270E-06 * phi * phi + 2.2578E-04 * phi + 5.9214E-03) * pp * pp + (-1.6419E-05 * phi * phi + -8.1776E-04 * phi + -3.2776E-02) * pp + 1.3734E-05 * phi * phi + 6.6125E-04 * phi + 4.5784E-02;
      }
      if(sec == 4) {
        dp = (-1.3141E-06 * phi * phi + 1.9648E-04 * phi + 7.6109E-03 - 0.006) * pp * pp + (8.0912E-06 * phi * phi + -8.2672E-04 * phi + -4.0495E-02 + 0.03) * pp + -3.1380E-06 * phi * phi + 6.2211E-04 * phi + 5.3361E-02 - 0.04;
      }
      if(sec == 5) {
        dp = (-5.4065E-06 * phi * phi + -1.6325E-05 * phi + 1.2269E-02 - 0.002) * pp * pp + (1.9512E-05 * phi * phi + 1.0228E-04 * phi + -6.2351E-02 + 0.01) * pp + -9.5023E-06 * phi * phi + -3.7997E-05 * phi + 7.1061E-02 - 0.02;
      }
      if(sec == 6) {
        dp = (-1.1882E-05 * phi * phi + 2.0101E-04 * phi + 1.1635E-02 - 0.01) * pp * pp + (5.8488E-05 * phi * phi + -6.4709E-04 * phi + -5.3833E-02 + 0.05) * pp + -4.4462E-05 * phi * phi + 3.7529E-04 * phi + 6.2130E-02 - 0.06;
      }
    }

    return dp / pp + 1;
  }


  double MomentumCorrection::EnergyLossInbending(vector_element_t const Px, vector_element_t const Py, vector_element_t const Pz, int const pid) const
  {

    // The following code is for the Energy Loss Corrections for the proton
    if(pid != particle::proton)
      return 1.0;

    double dE_loss = 0;
    auto pro       = sqrt(Px * Px + Py * Py + Pz * Pz);
    auto proth     = atan2(sqrt(Px * Px + Py * Py), Pz) * (180 / M_PI);

    // Inbending Energy Loss Correction //
    if(proth < 27) {
      dE_loss = exp(-2.739 - 3.932 * pro) + 0.002907;
    }
    else {
      dE_loss = exp(-1.2 - 4.228 * pro) + 0.007502;
    }
    return (pro + dE_loss) / pro;
  }


  double MomentumCorrection::EnergyLossOutbending(vector_element_t const Px, vector_element_t const Py, vector_element_t const Pz, int const pid) const
  {

    // The following code is for the Energy Loss Corrections for the proton
    if(pid != particle::proton)
      return 1.0;

    double dE_loss = 0;
    auto pro       = sqrt(Px * Px + Py * Py + Pz * Pz);
    auto proth     = atan2(sqrt(Px * Px + Py * Py), Pz) * (180 / M_PI);

    // Outbending Energy Loss Correction //
    if(proth > 27) {
      dE_loss = exp(-1.871 - 3.063 * pro) + 0.007517;
    }
    return (pro + dE_loss) / pro;
  }


  void MomentumCorrection::Stop()
  {
  }

}
