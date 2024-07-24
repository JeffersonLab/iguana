#!/usr/bin/env python3

# meson forbids custom environment variables; this script allows for it,
# but is only used as a last resort
# details: https://github.com/mesonbuild/meson/issues/9

from sys import argv, exit
from os import environ

if(len(argv) != 2):
    exit(1)
if argv[1] not in environ:
    exit(1)
print(environ[argv[1]])
