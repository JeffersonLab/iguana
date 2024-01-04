#!/usr/bin/env python3

import pyiguana
import sys

# include the header files that you need
pyiguana.include(
        'hipo4/reader.h',
        'iguana/algorithms/clas12/EventBuilderFilter.h',
        'iguana/algorithms/clas12/LorentzTransformer.h',
        )
# then import the bound namespaces (must be after including the headers)
from cppyy.gbl import hipo, iguana
# from here the syntax is analogous to the C++ example

inFile    = sys.argv[1]      if len(sys.argv)>1 else 'data.hipo'
numEvents = int(sys.argv[2]) if len(sys.argv)>2 else 3

reader = hipo.reader(inFile)
banks  = reader.getBanks(["REC::Particle", "REC::Calorimeter"]);

algo_eventbuilder_filter = iguana.clas12.EventBuilderFilter()
algo_lorentz_transformer = iguana.clas12.LorentzTransformer()

algo_eventbuilder_filter.SetOption('log',   'debug')
algo_lorentz_transformer.SetOption('log',   'debug')
algo_eventbuilder_filter.SetOption('pids',  [11, 211, -211])
algo_lorentz_transformer.SetOption('frame', 'mirror')

algo_eventbuilder_filter.Start()
algo_lorentz_transformer.Start()

iEvent = 0
while(reader.next(banks) and (numEvents==0 or iEvent < numEvents)):
    iEvent += 1

    particleBank = banks[0]
    particleBank.show()

    for row in range(particleBank.getRows()):

        pid = particleBank.getInt('pid', row)
        if(algo_eventbuilder_filter.Filter(pid)):

            px, py, pz, e = algo_lorentz_transformer.Transform(
                    particleBank.getFloat("px", row),
                    particleBank.getFloat("py", row),
                    particleBank.getFloat("pz", row),
                    0.0,
                    )

            print(f'Accepted PID {pid}:')
            print(f'  p_old = ({particleBank.getFloat("px", row)}, {particleBank.getFloat("py", row)}, {particleBank.getFloat("pz", row)})')
            print(f'  p_new = ({px}, {py}, {pz})')

algo_eventbuilder_filter.Stop()
algo_lorentz_transformer.Stop()
