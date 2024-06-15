#!/usr/bin/env python3

"""!
@begin_doc_example
@file iguana-example-bank-rows.py
@brief Python version of `iguana-example-bank-rows.cc`
@see `iguana-example-bank-rows.cc` for more information
@end_doc_example
@doxygen_off
"""

import pyiguana
import sys

# include the header files that you need
pyiguana.include(
        'hipo4/reader.h',
        'iguana/algorithms/clas12/EventBuilderFilter/Algorithm.h',
        'iguana/algorithms/clas12/MomentumCorrection/Algorithm.h',
        )
# then import the bound namespaces (must be after including the headers)
from cppyy.gbl import hipo, iguana
# from here the syntax is analogous to the C++ example

inFile    = sys.argv[1]      if len(sys.argv)>1 else 'data.hipo'
numEvents = int(sys.argv[2]) if len(sys.argv)>2 else 3

reader = hipo.reader(inFile)
banks  = reader.getBanks(["REC::Particle", "RUN::config"]);

algo_eventbuilder_filter = iguana.clas12.EventBuilderFilter()
algo_momentum_correction = iguana.clas12.MomentumCorrection()

algo_eventbuilder_filter.SetOption('log',   'debug')
algo_momentum_correction.SetOption('log',   'debug')
algo_eventbuilder_filter.SetOption('pids',  [11, 211, -211])

algo_eventbuilder_filter.Start()
algo_momentum_correction.Start()

iEvent = 0
while(reader.next(banks) and (numEvents==0 or iEvent < numEvents)):
    iEvent += 1

    particleBank = banks[0]
    configBank   = banks[1]
    particleBank.show()

    for row in particleBank.getRowList():

        pid = particleBank.getInt('pid', row)
        if(algo_eventbuilder_filter.Filter(pid)):

            sector = 1 # FIXME: get the sector number. The algorithm `clas12::SectorFinder` can do this, however
                       # it requires reading full `hipo::bank` objects, whereas this example is meant to demonstrate
                       # `iguana` usage operating _only_ on bank row elements

            px, py, pz, = algo_momentum_correction.Transform(
                    particleBank.getFloat("px", row),
                    particleBank.getFloat("py", row),
                    particleBank.getFloat("pz", row),
                    sector,
                    pid,
                    configBank.getFloat("torus", 0)
                    )

            print(f'Accepted PID {pid}:')
            print(f'  p_old = ({particleBank.getFloat("px", row)}, {particleBank.getFloat("py", row)}, {particleBank.getFloat("pz", row)})')
            print(f'  p_new = ({px}, {py}, {pz})')

algo_eventbuilder_filter.Stop()
algo_momentum_correction.Stop()

"""!@doxygen_on"""
