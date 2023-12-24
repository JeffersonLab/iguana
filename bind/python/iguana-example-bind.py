#!/usr/bin/env python3

import pyiguana
import sys

# include the header files that you need
pyiguana.include('hipo4/reader.h', 'iguana/algorithms/AlgorithmSequence.h')
# then import the bound namespaces (must be after including the headers)
from cppyy.gbl import hipo, iguana
# from here the syntax is analogous to the C++ example

inFile    = sys.argv[1]      if len(sys.argv)>1 else 'data.hipo'
numEvents = int(sys.argv[2]) if len(sys.argv)>2 else 3

reader = hipo.reader(inFile)
banks  = reader.getBanks(["REC::Particle", "REC::Calorimeter"]);

seq = iguana.AlgorithmSequence('pyiguana')
seq.Add('clas12::EventBuilderFilter')
seq.Add('clas12::LorentzTransformer')
seq.PrintSequence()

seq.SetOption('clas12::EventBuilderFilter', 'log',  'trace')
seq.SetOption('clas12::EventBuilderFilter', 'pids', [11, 211, -211])
seq.SetOption('clas12::EventBuilderFilter', 'testInt',   7)
seq.SetOption('clas12::EventBuilderFilter', 'testFloat', 42.3)

def prettyPrint(message, bank):
    print(f'{"="*30} {message} {"="*30}')
    bank.show()

iEvent = 0
seq.Start(banks)
while(reader.next(banks) and (numEvents==0 or iEvent < numEvents)):
    iEvent += 1
    prettyPrint("BEFORE", banks[0])
    seq.Run(banks)
    prettyPrint("AFTER", banks[0])

seq.Stop()
