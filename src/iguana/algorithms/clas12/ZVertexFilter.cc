#include "ZVertexFilter.h"

namespace iguana::clas12 {

  REGISTER_IGUANA_ALGORITHM(ZVertexFilter);

  void ZVertexFilter::Start(hipo::banklist& banks) {

    // Open INIReader to ZVertexFilter config files
    //atm using path from example folders to Config_files...
    //probably want to use an environment variable or something?
    INIReader inir("../../src/iguana/algorithms/clas12/Config_files/ZVertexFilter.ini");

    //In practice, get run period, if experiment and torus field setting from somewhere, sequencer??
    std::string runPeriod="fall_2018";
    std::string isExperiment="EXP"; //could also be SIM
    std::string fieldSetting="OUTBEND";

    //create section to read from config file
    std::string section=runPeriod+"/"+isExperiment+"/"+fieldSetting;

    //read the cut values at correct section. If reading values, default to -20.0,20.0
    std::vector<double> defaultValues = inir.readArray<double>(section, "vals", {-20.0,20.0});

    // define options, their default values, and cache them
    CacheOptionToSet("low&high",defaultValues, zvertex_lims);

    // cache expected bank indices
    CacheBankIndex(banks, "REC::Particle", b_particle);

  }


  void ZVertexFilter::Run(hipo::banklist& banks) const {

    // get the banks
    auto& particleBank = GetBank(banks, b_particle, "REC::Particle");

    // dump the bank
    ShowBank(particleBank, Logger::Header("INPUT PARTICLES"));

    // filter the input bank for requested PDG code(s)
    for(int row = 0; row < particleBank.getRows(); row++) {
      auto zvertex    = particleBank.getFloat("vz", row);
      auto accept = Filter(zvertex);
      if(!accept)
        MaskRow(particleBank, row);
      m_log->Debug("input vz {} -- accept = {}", zvertex, accept);
    }

    // dump the modified bank
    ShowBank(particleBank, Logger::Header("OUTPUT PARTICLES"));
  }


  bool ZVertexFilter::Filter(const double zvertex) const {
    std::set<double>::iterator it = zvertex_lims.begin();
    double low=*it;
    std::advance(it, 1);
    double high=*it;
    std::cout<<"low "<<low<<" high "<<high<<std::endl;
    return (zvertex >low) && (zvertex < high);
  }


  void ZVertexFilter::Stop() {
  }

}
