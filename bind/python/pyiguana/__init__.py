#!/usr/bin/env python3

import os, cppyy, pkgconfig

# add include and library directories to cppyy
for pkg in ['iguana'] + [p.split()[0] for p in pkgconfig.requires('iguana') if p!='']:
    if not pkgconfig.exists(pkg):
        raise Exception(f'failed to find "{pkg}.pc" in pkg-config path')
    pkg_info = {
            'includedir': os.path.realpath(pkgconfig.variables(pkg)['includedir']),
            'libdir':     os.path.realpath(pkgconfig.variables(pkg)['libdir']),
            }
    print(f'DEBUG: pyiguana - add package {pkg}: {pkg_info}')
    cppyy.add_include_path(pkg_info['includedir'])
    cppyy.add_library_path(pkg_info['libdir'])

# add libraries to cppyy
for lib in ['hipo4', 'IguanaServices', 'IguanaAlgorithms']:
    cppyy.load_library(lib)

# include header file(s)
def include(*headers):
    for header in headers:
        cppyy.include(header)
