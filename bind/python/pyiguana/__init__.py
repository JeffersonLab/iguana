#!/usr/bin/env python3

import cppyy, pkgconfig

# add include and library directories to cppyy
for pkg in ['iguana'] + [p.split()[0] for p in pkgconfig.requires('iguana') if p!='']:
    if not pkgconfig.exists(pkg):
        raise Exception(f'failed to find "{pkg}.pc" in pkg-config path')
    cppyy.add_include_path(pkgconfig.variables(pkg)['includedir'])
    cppyy.add_library_path(pkgconfig.variables(pkg)['libdir'])

# add libraries to cppyy
for lib in ['hipo4', 'IguanaServices', 'IguanaAlgorithms']:
    cppyy.load_library(lib)

# include header file(s)
def include(*headers):
    for header in headers:
        cppyy.include(header)
