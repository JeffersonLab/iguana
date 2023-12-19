#!/usr/bin/env python3

import cppyy
import PyIguana.AlgorithmSequence as iguana

cppyy.load_library('hipo4')
cppyy.include('hipo4/reader.h')
from cppyy.gbl import hipo

inFile    = 'data.hipo'
numEvents = 3

reader = hipo.reader(inFile)
cppyy.cppexec('std::vector<std::string> banks = { "REC::Particle", "REC::Calorimeter" };')
banks = reader.getBanks(cppyy.gbl.banks)

seq = iguana.AlgorithmSequence('PyIguana')
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
