#!/usr/bin/env python3

"""!
@begin_doc_example{python}
@file iguana_ex_python_00_run_functions.py
@brief Python version of `iguana_ex_cpp_00_run_functions.cc` (for more details, see this `.cc` file)
@end_doc_example
@doxygen_off
"""

import pyiguana
import sys

# include the header files that you need
pyiguana.include('hipo4/reader.h', 'iguana/algorithms/AlgorithmSequence.h')
# then import the bound namespaces (must be after including the headers)
from cppyy.gbl import hipo, iguana
# from here the syntax is analogous to the C++ example

# helper function to show a bank along with a header
def prettyPrint(message, bank):
    print(f'{"="*30} {message} {"="*30}')
    bank.show()

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

# get bank index, for each bank we want to use after Iguana algorithms run
b_particle = hipo.getBanklistIndex(banks, "REC::Particle")

# iguana algorithm sequence
seq = iguana.AlgorithmSequence('pyiguana')
seq.Add('clas12::EventBuilderFilter')
seq.Add('clas12::SectorFinder')
seq.Add('clas12::MomentumCorrection')
seq.PrintSequence()

# set log levels
seq.SetOption('clas12::EventBuilderFilter', 'log', 'debug')
seq.SetOption('clas12::MomentumCorrection', 'log', 'debug')

# set algorithm options (overrides configuration files)
seq.SetOption('clas12::EventBuilderFilter', 'pids', [11, 211, -211])

# start the algorithms
seq.Start(banks)

# run the algorithm sequence on each event
iEvent = 0
while(reader.next(banks) and (numEvents==0 or iEvent < numEvents)):
    iEvent += 1
    prettyPrint("BEFORE", banks[b_particle])
    seq.Run(banks)
    prettyPrint("AFTER", banks[b_particle])

# stop algorithms
seq.Stop()

"""!@doxygen_on"""
