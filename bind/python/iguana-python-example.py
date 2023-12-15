#!/usr/bin/env python3

import PyIguana.AlgorithmSequence as iguana

seq = iguana.AlgorithmSequence('PyIguana')

algos = [
        'clas12::EventBuilderFilter',
        'clas12::LorentzTransformer',
        ]
for algo in algos:
    seq.Add(algo)
seq.PrintSequence()

# # TODO: need to marshal to std::variant; config files are a possible workaround here
# for algo in algos:
#     seq.SetOption(algo, 'log', 'trace')
