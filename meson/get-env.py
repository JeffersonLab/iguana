#!/usr/bin/env python3

# Meson forbids custom environment variables; however, our primary deployment
# platform uses Environment Modules, which heavily relies on environment
# variables. Assuming most of our users don't want to worry about this
# constraint, this script grants Meson access to certain environment variables
# from dependencies that need them.
#
# Build options are provided which take prioroty over such environment
# variables; usage of this script should ONLY be a last resort.
#
# Details: https://github.com/mesonbuild/meson/issues/9

from sys import argv, exit
from os import environ

if(len(argv) != 2):
    exit(1)
if argv[1] not in environ:
    exit(1)
print(environ[argv[1]])
