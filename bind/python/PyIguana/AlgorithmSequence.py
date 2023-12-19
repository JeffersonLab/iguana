#!/usr/bin/env python3

import os
import cppyy

iguana_include_path = os.environ.get('IGUANA_INCLUDE_PATH')
if iguana_include_path is not None:
    for path in iguana_include_path.split(':'):
        cppyy.add_include_path(path)

cppyy.load_library('IguanaAlgorithms')
cppyy.include('iguana/algorithms/AlgorithmSequence.h')

from cppyy.gbl import iguana
from cppyy.gbl.iguana import AlgorithmSequence
