#!/usr/bin/env python3

from configparser import ConfigParser
import argparse, os, sys, textwrap, subprocess

# constants
SYSTEM_ASSUMPTION     = 'assume system installation'
SEPARATOR             = '-'*50
PKGCONFIG_RELOCATABLE = True
LIBDIR                = 'lib'

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
parser_deps.add_argument( '--hipo', default=SYSTEM_ASSUMPTION, type=str, help='path to `hipo` installation')
parser_deps.add_argument( '--fmt', default=SYSTEM_ASSUMPTION, type=str, help='path to `fmt` installation')
parser_build = parser.add_argument_group('build settings')
parser_build.add_argument( '--prefix', default='iguana', type=str, help='iguana installation prefix')
parser_build.add_argument( '--debug', default=False, action=argparse.BooleanOptionalAction, help='enable debugging symbols or not; optimization level is 0 if this is used, otherwise it is 3 (see also --buildtype)')
parser_build.add_argument( '--examples', default=False, action=argparse.BooleanOptionalAction, help='build examples or not')
parser_build.add_argument( '--documentation', default=False, action=argparse.BooleanOptionalAction, help='generate API documentation or not')
parser_build = parser.add_argument_group('bindings')
parser_build.add_argument( '--python', default=False, action=argparse.BooleanOptionalAction, help='generate Python bindings or not')
parser_build = parser.add_argument_group('advanced settings')
parser_build.add_argument( '--build', default='build-iguana', type=str, help='iguana buildsystem directory')
parser_build.add_argument( '--ini', default='build-iguana.ini', type=str, help='name of the output config INI file')
parser_build.add_argument( '--buildtype', default='release', type=str, help='specify meson built-in option `buildtype`')
args = parser.parse_args()

# get prefix and source absolute paths
installDir = os.path.realpath(args.prefix)
sourceDir  = os.path.dirname(os.path.realpath(__file__))

# set dependency paths
cmake_prefix_path = set()
cmake_deps        = set()
pkg_config_path   = set()
pkg_config_deps   = set()
if(args.hipo != SYSTEM_ASSUMPTION):
    pkg_config_path.add(os.path.realpath(args.hipo) + '/lib/pkgconfig')
    pkg_config_deps.add('hipo')
if(args.fmt != SYSTEM_ASSUMPTION):
    pkg_config_path.add(os.path.realpath(args.fmt) + '/lib/pkgconfig')
    pkg_config_deps.add('fmt')

# set the buildtype
buildtype = 'debug' if args.debug else args.buildtype

# return an array of strings for meson's INI parsing
def meson_string_array(arr):
    contents = ','.join(map(lambda s: f'\'{s}\'', arr))
    return f'[{contents}]'

# generate the INI file
config = ConfigParser(allow_no_value=True)
config.add_section('built-in options')
if(len(cmake_prefix_path) > 0):
    config.set('built-in options', '; path to dependencies: ' + ','.join(cmake_deps))
    config.set('built-in options', 'cmake_prefix_path', meson_string_array(cmake_prefix_path))
if(len(pkg_config_path) > 0):
    config.set('built-in options', '; path to dependencies: ' + ','.join(pkg_config_deps))
    config.set('built-in options', 'pkg_config_path', meson_string_array(pkg_config_path))
config.set('built-in options', '; constant settings (do not edit)')
config.set('built-in options', 'libdir', f'\'{LIBDIR}\'') # make all systems use LIBDIR
config.set('built-in options', 'pkgconfig.relocatable', f'{PKGCONFIG_RELOCATABLE}')
config.set('built-in options', '; build settings')
config.set('built-in options', 'prefix', f'\'{installDir}\'')
config.set('built-in options', 'buildtype', f'\'{buildtype}\'')
config.set('built-in options', 'examples', f'{args.examples}')
config.set('built-in options', 'documentation', f'{args.documentation}')
config.set('built-in options', '; bindings')
config.set('built-in options', 'bind_python', f'{args.python}')

# write the INI file
with open(args.ini, 'w') as fp:
    config.write(fp)
print(f'Wrote build configuration file {args.ini}:')
print(SEPARATOR)
with open(args.ini, 'r') as fp:
    print(fp.read())
print(SEPARATOR)

# generate the installation script
installScript = 'install-iguana.sh'
with open(installScript, 'w') as fp:
    fp.write(textwrap.dedent(f'''\
    #!/bin/bash
    set -e
    meson setup --native-file {args.ini} {args.build} {sourceDir}
    meson install -C {args.build}
    '''))
os.chmod(installScript, 0o744)
print(f'''
Generated installation script {installScript}
To proceed with iguana installation, run:

      ./{installScript}
''')
