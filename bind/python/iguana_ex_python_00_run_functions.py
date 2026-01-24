#!/usr/bin/env python3

"""!
@begin_doc_example{python}
@file iguana_ex_python_00_run_functions.py
@brief Python version of `iguana_ex_cpp_00_run_functions.cc` (for more details, see this `.cc` file)
@note You may need to run this example using `stdbuf -o0` (preceding your command) if the output appears to be jumbled
@end_doc_example
@doxygen_off
"""

import pyiguana
import sys
import math

# include the header files that you need
pyiguana.include('hipo4/reader.h', 'iguana/algorithms/AlgorithmSequence.h')
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
    "RUN::config",
    "REC::Particle",
    "REC::Calorimeter",
    "REC::Track",
    "REC::Scintillator"])

# iguana algorithm sequence
# NOTE: the order that they are added to the sequence here will be the same order in which they will be run
seq = iguana.AlgorithmSequence('pyiguana')
seq.Add('clas12::EventBuilderFilter') # filter by Event Builder PID (a filter algorithm)
seq.Add('clas12::SectorFinder') # get the sector for each particle (a creator algorithm)
seq.Add('clas12::rga::MomentumCorrection') # momentum corrections (a transformer algorithm)
# seq.PrintSequence()

# configure algorithms with a custom YAML file
# - in practice you can put your config file(s) where you want
# - for this example, we use a YAML file installed alongside iguana (copied from `./config/config.yaml`)
config_file = iguana.ConfigFileReader.GetConfigInstallationPrefix() + '/examples/config.yaml'
# print the file name (so you can open it to see)
print(f'CONFIG FILE: {config_file}')
# use this configuration for all algorithms in the sequence
seq.SetConfigFileForEachAlgorithm(config_file)
# alternatively: use this configuration for the algorithm that needs it
# seq.Get("clas12::EventBuilderFilter").SetConfigFile(config_file)

# start the algorithms
seq.Start(banks)

# get bank index, for each bank we want to use after Iguana algorithms run
# NOTE: new banks from creator algorithms are initialized by `Start`
b_config   = iguana.tools.GetBankIndex(banks, 'RUN::config')
b_particle = iguana.tools.GetBankIndex(banks, 'REC::Particle')
b_sector   = seq.GetCreatedBankIndex(banks, "clas12::SectorFinder") # newly created bank; string parameter is algorithm name, not bank name

# run the algorithm sequence on each event
iEvent = 0
while(reader.next(banks) and (numEvents==0 or iEvent < numEvents)):
    iEvent += 1

    # print the event number
    # NOTE: we use 'flush=True' in Python `print` calls here, otherwise these `print`
    # calls will be out-of-order with respect to C++ printouts coming from, e.g.,
    # Iguana and `hipo::bank::show()` (they use different buffers)
    print(f'===== EVENT {banks[b_config].getInt("event", 0)} =====', flush=True)

    # print the particle bank before Iguana algorithms
    print('----- BEFORE IGUANA -----', flush=True)
    banks[b_particle].show() # the original particle bank

    # run the sequence of Iguana algorithms
    seq.Run(banks)

    # print the banks after Iguana algorithms
    print('----- AFTER IGUANA -----', flush=True)
    banks[b_particle].show() # the filtered particle bank, with corrected momenta
    banks[b_sector].show()   # the new sector bank

    # print a table; first the header
    print('----- Analysis Particles -----', flush=True)
    print(f'  {"row == pindex":<20} {"PDG":<20} {"|p|":<20} {"sector":<20}', flush=True)
    # then print a row for each particle
    # - use the `banks[b_particle].getRowList()` method to loop over the bank rows that PASS the filter; note
    #   that it needs to be converted via `list()` in order to be iterated
    # - if you'd rather loop over ALL bank rows, iterate from `i=0` up to `i < banks[b_particle].getRows()` instead
    for row in list(banks[b_particle].getRowList()):
        p = math.hypot(
            banks[b_particle].getFloat('px', row),
            banks[b_particle].getFloat('py', row),
            banks[b_particle].getFloat('pz', row))
        pdg = banks[b_particle].getInt('pid', row)
        sector = banks[b_sector].getInt('sector', row)
        print(f'  {row:<20} {pdg:<20} {p:<20.3f} {sector:<20}', flush=True)
    print(flush=True)

# stop algorithms
seq.Stop()

"""!@doxygen_on"""
