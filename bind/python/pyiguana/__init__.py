#!/usr/bin/env python3

import os, cppyy, pkgconfig

# read iguana pkg-config
PKG = 'iguana'
if not pkgconfig.exists(PKG):
    raise Exception(f'failed to find pkg-config package "{PKG}"')
pkg_vars = pkgconfig.variables(PKG)

# add include dirs to cppyy
for var in ['includedir', 'dep_includedirs']:
    include_path = pkg_vars[var]
    for path in include_path.split(':'):
        cppyy.add_include_path(path)

# add libraries to cppyy
for lib in ['hipo4', 'IguanaServices', 'IguanaAlgorithms']:
  cppyy.load_library(lib)

# include header file(s)
def include(*headers):
    for header in headers:
        cppyy.include(header)
