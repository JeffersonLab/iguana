#!/usr/bin/env python3

import os, cppyy, pkgconfig

# add include directories to cppyy
for pkg in ['hipo4', 'iguana']:
    if not pkgconfig.exists(pkg):
        raise Exception(f'failed to find "{pkg}.pc" in pkg-config path')
    cppyy.add_include_path(pkgconfig.variables(pkg)['includedir'])

# add libraries to cppyy
for lib in ['hipo4', 'IguanaServices', 'IguanaAlgorithms']:
  cppyy.load_library(lib)

# include header file(s)
def include(*headers):
    for header in headers:
        cppyy.include(header)
