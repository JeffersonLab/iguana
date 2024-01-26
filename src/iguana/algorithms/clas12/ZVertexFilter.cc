#include "ZVertexFilter.h"

namespace iguana::clas12
{

    REGISTER_IGUANA_ALGORITHM(ZVertexFilter);

    void ZVertexFilter::Start(hipo::banklist &banks)
    {
        
        //Eventually need to find a good location for files
        // and a good way of passing in run numbers and pid values
        int runnb = 4768; //default to RG-A fall2018 inbending for now
        int pid=0; //no PID needed for this filter
        YAMLReader yamlr("../../src/iguana/algorithms/clas12/Config_files/ZVertexFilter.yaml");

        //Read YAML config file with cuts for a given run number.
        std::vector<double> defaultValues = yamlr.findKeyAtRunAndPIDVector<double>("cuts","runs","pid", "vals", runnb,pid, {-20.0, 20.0});

        // define options, their default values, and cache them
        CacheOption("low", defaultValues.at(0), zvertex_low);
        CacheOption("high", defaultValues.at(1), zvertex_high);

        // cache expected bank indices
        CacheBankIndex(banks, "REC::Particle", b_particle);
    }

    void ZVertexFilter::Run(hipo::banklist &banks) const
    {

        // get the banks
        auto &particleBank = GetBank(banks, b_particle, "REC::Particle");

        // dump the bank
        ShowBank(particleBank, Logger::Header("INPUT PARTICLES"));

        // filter the input bank for requested PDG code(s)
        for (int row = 0; row < particleBank.getRows(); row++)
        {
            auto zvertex = particleBank.getFloat("vz", row);
            auto accept = Filter(zvertex);
            if (!accept)
                MaskRow(particleBank, row);
            m_log->Debug("input vz {} -- accept = {}", zvertex, accept);
        }

        // dump the modified bank
        ShowBank(particleBank, Logger::Header("OUTPUT PARTICLES"));
    }

    bool ZVertexFilter::Filter(const double zvertex) const
    {
        //std::cout << "low " << zvertex_low << " high " << zvertex_high << std::endl;
        return (zvertex > zvertex_low) && (zvertex < zvertex_high);
    }

    void ZVertexFilter::Stop()
    {
    }

}
