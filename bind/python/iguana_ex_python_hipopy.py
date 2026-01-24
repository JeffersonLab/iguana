#!/usr/bin/env python3

"""!
@begin_doc_example{python}
@file iguana_ex_python_hipopy.py
@brief Python iguana example using HIPOPy: https://github.com/mfmceneaney/hipopy
@note You may need to run this example using `stdbuf -o0` (preceding your command) if the output appears to be jumbled
@end_doc_example
@doxygen_off
"""
#NOTE: You must import hipopy, which imports hipopybind, BEFORE any cppyy libraries.
import hipopy.hipopy as hp
import pyiguana
import sys
import os.path as osp

# include the header files that you need
pyiguana.include(
        'hipo4/reader.h',
        'iguana/algorithms/clas12/EventBuilderFilter/Algorithm.h',
        'iguana/algorithms/clas12/SectorFinder/Algorithm.h',
        'iguana/algorithms/clas12/rga/MomentumCorrection/Algorithm.h',
        )
# then import the bound namespaces (must be after including the headers)
from cppyy.gbl import hipo, iguana
# from here the syntax is analogous to the C++ example
if len(sys.argv)<=1:
    print('Usage: python3',osp.basename(__file__),' <inFile> <step> <nbatches>')
    sys.exit(0)
inFile   = sys.argv[1]      if len(sys.argv)>1 else 'data.hipo'
step     = int(sys.argv[2]) if len(sys.argv)>2 else 3
nbatches = int(sys.argv[3]) if len(sys.argv)>3 else 1

banks = [
    "REC::Particle",
    "RUN::config",
    "REC::Track",
    "REC::Calorimeter",
    "REC::Scintillator",
]

# create the algorithms
algo_eventbuilder_filter = iguana.clas12.EventBuilderFilter() # filter by Event Builder PID (a filter algorithm)
algo_sector_finder       = iguana.clas12.SectorFinder() # get the sector for each particle (a creator algorithm)
algo_momentum_correction = iguana.clas12.rga.MomentumCorrection() # momentum corrections (a transformer algorithm)

# configure algorithms with a custom YAML file
config_file = iguana.ConfigFileReader.GetConfigInstallationPrefix() + '/examples/config.yaml'
algo_eventbuilder_filter.SetConfigFile(config_file)
algo_sector_finder.SetConfigFile(config_file)
algo_momentum_correction.SetConfigFile(config_file)

# start the algorithms
algo_eventbuilder_filter.Start()
algo_sector_finder.Start()
algo_momentum_correction.Start()

# run the algorithms on each event
for iBatch, batch in enumerate(hp.iterate([inFile],banks=banks,step=step)):

    for iEvent, pxs in enumerate(batch['REC::Particle_px']):

        # verbose printout
        print(f'evnum = {batch["RUN::config_event"][iEvent][0]}')
        # print(f'iBatch={iBatch}, iEvent={iEvent}')
        # for key in batch:
        #     print(key,':',batch[key][iEvent])

        # loop over particles
        for row, _px in enumerate(pxs):
            pid = batch['REC::Particle_pid'][iEvent][row]

            # check the PID with EventBuilderFilter
            if(algo_eventbuilder_filter.Filter(pid)):

                # get the sector for this particle; this is using a vector action function, so
                # many of its arguments are full arrays
                sector = algo_sector_finder.GetStandardSector(
                        batch['REC::Track_sector'][iEvent],
                        batch['REC::Track_pindex'][iEvent],
                        batch['REC::Calorimeter_sector'][iEvent],
                        batch['REC::Calorimeter_pindex'][iEvent],
                        batch['REC::Scintillator_sector'][iEvent],
                        batch['REC::Scintillator_pindex'][iEvent],
                        row)

                # correct the particle momentum
                p_corrected = algo_momentum_correction.Transform(
                    batch['REC::Particle_px'][iEvent][row],
                    batch['REC::Particle_py'][iEvent][row],
                    batch['REC::Particle_pz'][iEvent][row],
                    sector,
                    pid,
                    batch['RUN::config_torus'][iEvent][0]
                )

                # then print the result
                print(f'Analysis Particle PDG = {pid}')
                print(f'  sector = {sector}')
                print(f'  p_old = ({batch["REC::Particle_px"][iEvent][row]:11.5f}, {batch["REC::Particle_py"][iEvent][row]:11.5f}, {batch["REC::Particle_pz"][iEvent][row]:11.5f})')
                print(f'  p_new = ({p_corrected.px:11.5f}, {p_corrected.py:11.5f}, {p_corrected.pz:11.5f})')

    # End iteration if maximum number of batches reached
    if (iBatch+1>=nbatches): break

# stop the algorithms
algo_eventbuilder_filter.Stop()
algo_sector_finder.Stop()
algo_momentum_correction.Stop()

"""!@doxygen_on"""
