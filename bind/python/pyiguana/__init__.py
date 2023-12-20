#!/usr/bin/env python3

import os, cppyy

# add include dirs to cpppyy
iguana_include_path = os.environ.get('IGUANA_INCLUDE_PATH')
if iguana_include_path is not None:
    for path in iguana_include_path.split(':'):
        cppyy.add_include_path(path)

# add libraries to cppyy
[ cppyy.load_library(lib) for lib in [ 'hipo4', 'IguanaServices', 'IguanaAlgorithms' ]]

# include header file(s)
def include(*headers):
    for header in headers:
        cppyy.include(header)
