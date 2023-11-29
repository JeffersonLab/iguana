#!/usr/bin/env python

from configparser import ConfigParser
import argparse, os, sys, textwrap

SYSTEM_ASSUMPTION = 'assume system installation'
SEPARATOR = '-'*50

# parse user options
class Formatter(argparse.ArgumentDefaultsHelpFormatter, argparse.RawDescriptionHelpFormatter): pass
parser = argparse.ArgumentParser(
        usage = f'{sys.argv[0]} [OPTION]...',
        description = textwrap.dedent('''
        description:
          Generate a configuration file with build settings for iguana
        '''),
        formatter_class = Formatter
        )
parser_deps = parser.add_argument_group('dependency installation paths')
parser_deps.add_argument( '--hipo', default=SYSTEM_ASSUMPTION, type=str, help='path to `hipo` installation' )
parser_deps.add_argument( '--fmt',  default=SYSTEM_ASSUMPTION, type=str, help='path to `fmt` installation'  )
parser_build = parser.add_argument_group('iguana build settings')
parser_build.add_argument( '--prefix', default='iguana',           type=str, help='iguana installation prefix'         )
parser_build.add_argument( '--build',  default='build-iguana',     type=str, help='iguana buildsystem directory'       )
parser_build.add_argument( '--ini',    default='build-iguana.ini', type=str, help='name of the output config INI file' )
args = parser.parse_args()

# set dependency paths
cmake_prefix_path = []
pkg_config_path = []
if(args.hipo != SYSTEM_ASSUMPTION):
    cmake_prefix_path.append(os.path.realpath(args.hipo))
if(args.fmt != SYSTEM_ASSUMPTION):
    pkg_config_path.append(os.path.realpath(args.fmt) + '/lib/pkgconfig')

# return an array of strings for meson's INI parsing
def meson_string_array(arr):
    contents = ','.join(map(lambda s: f'\'{s}\'', arr))
    return f'[{contents}]'

# generate the INI file
config = ConfigParser(allow_no_value=True)
config.add_section('built-in options')
config.set('built-in options', '; dependency paths')
config.set('built-in options', 'cmake_prefix_path', meson_string_array(cmake_prefix_path))
config.set('built-in options', 'pkg_config_path', meson_string_array(pkg_config_path))
config.set('built-in options', '; installation settings')
config.set('built-in options', 'prefix', f'\'{os.path.realpath(args.prefix)}\'')
with open(args.ini, 'w') as fp:
    config.write(fp)
print(f'Wrote build configuration file {args.ini}:')
print(SEPARATOR)
with open(args.ini, 'r') as fp:
    print(fp.read())
print(SEPARATOR)

# generate the installation script
sourceDir = os.path.dirname(os.path.realpath(__file__))
installScript = 'install-iguana.sh'
with open(installScript, 'w') as fp:
    fp.write(textwrap.dedent(f'''\
    #!/bin/bash
    set -e

    # setup
    if [ ! -d {args.build} ]; then
      meson setup --native-file {args.ini} {args.build} {sourceDir}
    fi

    # compile and install
    meson install -C {args.build}
    '''))
os.chmod(installScript, 0o744)
print(f'''
Generated installation script {installScript}
To proceed with iguana installation, run:

      ./{installScript}
''')
