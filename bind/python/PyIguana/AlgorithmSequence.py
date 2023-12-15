#!/usr/bin/env python3

import os
import cppyy

# TODO: use jinja to set these paths, rather than relying on env var IGUANA;
#       we should NOT need $IGUANA to be set
for prefix in map(lambda d: os.environ.get(d), ['IGUANA', 'HIPO']):
    if prefix is not None:
        cppyy.add_include_path(f'{prefix}/include')

cppyy.load_library('IguanaAlgorithms')
cppyy.include('iguana/algorithms/AlgorithmSequence.h')

from cppyy.gbl import iguana
from cppyy.gbl.iguana import AlgorithmSequence
