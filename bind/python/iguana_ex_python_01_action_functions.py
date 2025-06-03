#!/usr/bin/env python3

"""!
@begin_doc_example{python}
@file iguana_ex_python_01_action_functions.py
@brief Python version of `iguana_ex_cpp_01_action_functions.cc` (for more details, see this `.cc` file)
@end_doc_example
@doxygen_off
"""

import pyiguana
import sys

# include the header files that you need
pyiguana.include(
        'hipo4/reader.h',
        'iguana/algorithms/clas12/EventBuilderFilter/Algorithm.h',
        'iguana/algorithms/clas12/SectorFinder/Algorithm.h',
        'iguana/algorithms/clas12/MomentumCorrection/Algorithm.h',
        )
# then import the bound namespaces (must be after including the headers)
from cppyy.gbl import hipo, iguana
# from here the syntax is analogous to the C++ example

# parse arguments
inFile    = sys.argv[1]      if len(sys.argv)>1 else 'data.hipo'
numEvents = int(sys.argv[2]) if len(sys.argv)>2 else 3

# read input file
reader = hipo.reader(inFile)

# set list of banks to be read
banks  = reader.getBanks([
    "REC::Particle",
    "RUN::config",
    "REC::Track",
    "REC::Calorimeter",
    "REC::Scintillator",
]);

# get bank index, for each bank we want to use after Iguana algorithms run
b_particle     = hipo.getBanklistIndex(banks, "REC::Particle")
b_config       = hipo.getBanklistIndex(banks, "RUN::config")
b_track        = hipo.getBanklistIndex(banks, "REC::Track");
b_calorimeter  = hipo.getBanklistIndex(banks, "REC::Calorimeter");
b_scintillator = hipo.getBanklistIndex(banks, "REC::Scintillator");

# create the algorithms
algo_eventbuilder_filter = iguana.clas12.EventBuilderFilter()
algo_sector_finder       = iguana.clas12.SectorFinder()
algo_momentum_correction = iguana.clas12.MomentumCorrection()

# set log levels
algo_eventbuilder_filter.SetOption('log', 'debug')
algo_sector_finder.SetOption('log', 'debug')
algo_momentum_correction.SetOption('log', 'debug')

# set algorithm options
algo_eventbuilder_filter.SetOption('pids',  [11, 211, -211])

# start the algorithms
algo_eventbuilder_filter.Start()
algo_sector_finder.Start()
algo_momentum_correction.Start()

# run the algorithms on each event
iEvent = 0
while(reader.next(banks) and (numEvents==0 or iEvent < numEvents)):
    iEvent += 1

    # get the banks for this event
    particleBank     = banks[b_particle]
    configBank       = banks[b_config]
    trackBank        = banks[b_track]
    calorimeterBank  = banks[b_calorimeter]
    scintillatorBank = banks[b_scintillator]

    # we'll need information from all the rows of REC::Track,Calorimeter,Scintilator,
    # in order to get the sector information for each particle
    # NOTE: not giving a row number argument to these `get*` functions means they will return Arrays of all rows' values
    # FIXME: these are not the right integer-type accessors (need `getShort` and `getByte`); see https://github.com/gavalian/hipo/issues/72
    trackBank_sectors         = trackBank.getInt("pindex");
    trackBank_pindices        = trackBank.getInt("sector");
    calorimeterBank_sectors   = calorimeterBank.getInt("pindex");
    calorimeterBank_pindices  = calorimeterBank.getInt("sector");
    scintillatorBank_sectors  = scintillatorBank.getInt("pindex");
    scintillatorBank_pindices = scintillatorBank.getInt("sector");

    # show the particle bank
    particleBank.show()

    # loop over bank rows
    for row in particleBank.getRowList():

        # check the PID with EventBuilderFilter
        pid = particleBank.getInt('pid', row)
        if(algo_eventbuilder_filter.Filter(pid)):

            # get the sector for this particle; this is using a vector action function, so
            # many of its arguments are arrays
            sector = algo_sector_finder.GetStandardSector(
                    trackBank_sectors,
                    trackBank_pindices,
                    calorimeterBank_sectors,
                    calorimeterBank_pindices,
                    scintillatorBank_sectors,
                    scintillatorBank_pindices,
                    row)

            # correct the particle momentum
            p_corrected = algo_momentum_correction.Transform(
                    particleBank.getFloat("px", row),
                    particleBank.getFloat("py", row),
                    particleBank.getFloat("pz", row),
                    sector,
                    pid,
                    configBank.getFloat("torus", 0)
                    )

            # then print the result
            print(f'Accepted PID {pid}:')
            print(f'  p_old = ({particleBank.getFloat("px", row)}, {particleBank.getFloat("py", row)}, {particleBank.getFloat("pz", row)})')
            print(f'  p_new = ({p_corrected.px}, {p_corrected.py}, {p_corrected.pz})')

# stop the algorithms
algo_eventbuilder_filter.Stop()
algo_momentum_correction.Stop()

"""!@doxygen_on"""
