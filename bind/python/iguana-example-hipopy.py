#!/usr/bin/env python3

"""!
@begin_doc_example
@file iguana-example-hipopy.py
@brief Python iguana example using HIPOPy: https://github.com/mfmceneaney/hipopy
@end_doc_example
@doxygen_off
"""

import pyiguana
import sys
import hipopy.hipopy as hp
import os.path as osp

# include the header files that you need
pyiguana.include(
        'hipo4/reader.h',
        'iguana/algorithms/clas12/EventBuilderFilter/Algorithm.h',
        'iguana/algorithms/clas12/MomentumCorrection/Algorithm.h',
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

banks   = ["REC::Particle", "RUN::config"]

algo_eventbuilder_filter = iguana.clas12.EventBuilderFilter()
algo_momentum_correction = iguana.clas12.MomentumCorrection()

algo_eventbuilder_filter.SetOption('log',   'debug')
algo_momentum_correction.SetOption('log',   'debug')
algo_eventbuilder_filter.SetOption('pids',  [11, 211, -211])

algo_eventbuilder_filter.Start()
algo_momentum_correction.Start()

for iBatch, batch in enumerate(hp.iterate([inFile],banks=banks,step=step)):

    for iEvent, pxs in enumerate(batch['REC::Particle_px']):
        for key in batch:
            print(key,':',batch[key][iEvent])

        for row, _px in enumerate(pxs):
            pid = batch['REC::Particle_pid'][iEvent][row]
        
            if(algo_eventbuilder_filter.Filter(pid)):
                sector = 1 # FIXME: get the sector number. The algorithm `clas12::SectorFinder` can do this, however
                        # it requires reading full `hipo::bank` objects, whereas this example is meant to demonstrate
                        # `iguana` usage operating _only_ on bank row elements

                px, py, pz, = algo_momentum_correction.Transform(
                    batch['REC::Particle_px'][iEvent][row],
                    batch['REC::Particle_py'][iEvent][row],
                    batch['REC::Particle_pz'][iEvent][row],
                    sector,
                    pid,
                    batch['RUN::config_torus'][iEvent][0]
                )

                print(f'Accepted PID {pid}:')
                print(f'  p_old = ({batch["REC::Particle_px"][iEvent][row]}, {batch["REC::Particle_py"][iEvent][row]}, {batch["REC::Particle_pz"][iEvent][row]})')
                print(f'  p_new = ({px}, {py}, {pz})')

    # End iteration if maximum number of batches reached
    if (iBatch>=nbatches): break

algo_eventbuilder_filter.Stop()
algo_momentum_correction.Stop()

"""!@doxygen_on"""
