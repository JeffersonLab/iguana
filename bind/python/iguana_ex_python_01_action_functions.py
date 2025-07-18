#!/usr/bin/env python3

"""!
@begin_doc_example{python}
@file iguana_ex_python_01_action_functions.py
@brief Python version of `iguana_ex_cpp_01_action_functions.cc` (for more details, see this `.cc` file)
@note You may need to run this example using `stdbuf -o0` (preceding your command) if the output appears to be jumbled
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
banks = reader.getBanks([
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
algo_eventbuilder_filter.SetOption('log', 'info')
algo_sector_finder.SetOption('log', 'info')
algo_momentum_correction.SetOption('log', 'info')

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

    # show the particle bank
    # particleBank.show()

    # print the event number
    print(f'evnum = {configBank.getInt("event",0)}')

    # we'll need information from all the rows of REC::Track,Calorimeter,Scintilator,
    # in order to get the sector information for each particle
    # FIXME: there are vectorized accessors, but we cannot use them yet; see https://github.com/gavalian/hipo/issues/72
    #        until then, we fill lists manually
    trackBank_sectors = []
    trackBank_pindices = []
    calorimeterBank_sectors = []
    calorimeterBank_pindices = []
    scintillatorBank_sectors = []
    scintillatorBank_pindices = []
    for r in trackBank.getRowList():
        trackBank_sectors.append(trackBank.getByte("sector", r))
        trackBank_pindices.append(trackBank.getShort("pindex", r))
    for r in calorimeterBank.getRowList():
        calorimeterBank_sectors.append(calorimeterBank.getByte("sector", r))
        calorimeterBank_pindices.append(calorimeterBank.getShort("pindex", r))
    for r in scintillatorBank.getRowList():
        scintillatorBank_sectors.append(scintillatorBank.getByte("sector", r))
        scintillatorBank_pindices.append(scintillatorBank.getShort("pindex", r))

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
            print(f'Analysis Particle PDG = {pid}')
            print(f'  sector = {sector}')
            print(f'  p_old = ({particleBank.getFloat("px", row):11.5f}, {particleBank.getFloat("py", row):11.5f}, {particleBank.getFloat("pz", row):11.5f})')
            print(f'  p_new = ({p_corrected.px:11.5f}, {p_corrected.py:11.5f}, {p_corrected.pz:11.5f})')

# stop the algorithms
algo_eventbuilder_filter.Stop()
algo_sector_finder.Stop()
algo_momentum_correction.Stop()

"""!@doxygen_on"""
