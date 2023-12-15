#!/usr/bin/env python3

import cppyy

# TODO: use jinja to set these paths
cppyy.add_include_path('iguana/include')
cppyy.add_include_path('../install/include') # for HIPO

cppyy.load_library('IguanaAlgorithms')
cppyy.include('iguana/algorithms/AlgorithmSequence.h')

from cppyy.gbl import iguana
from cppyy.gbl.iguana import AlgorithmSequence
