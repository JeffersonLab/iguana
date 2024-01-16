#!/usr/bin/env python3

import argparse, os, sys, textwrap

# constants
SYSTEM_ASSUMPTION = 'assume system installation'
SEPARATOR         = '-'*50

# parse user options
class Formatter(argparse.ArgumentDefaultsHelpFormatter, argparse.RawDescriptionHelpFormatter): pass
parser = argparse.ArgumentParser(
    usage = f'{sys.argv[0]} [OPTION]...',
    description = textwrap.dedent('''
        description:
          Generate build options for resolving dependencies
        '''),
    formatter_class = Formatter
)
def define_output_opt(parser, name, desc):
    mutex_group = parser.add_mutually_exclusive_group(required=False)
    mutex_group.add_argument(f'--{name}', dest=name, action='store_true', help=desc)
    parser.set_defaults(**{name:False})
parser_deps = parser.add_argument_group('dependency installation paths')
parser_deps.add_argument( '--hipo', default=SYSTEM_ASSUMPTION, type=str, help='path to `hipo` installation')
parser_deps.add_argument( '--fmt', default=SYSTEM_ASSUMPTION, type=str, help='path to `fmt` installation')
parser_output = parser.add_argument_group('output control: by default this script is verbose; choose one of these for terse output')
define_output_opt(parser_output, 'cli', 'only print the `meson` CLI options')
define_output_opt(parser_output, 'ini', 'only print the lines for the INI file')
args = parser.parse_args()

# verbosity
verbose = not ( args.cli or args.ini )
def print_verbose(message):
    if(verbose):
        print(message)

##################################################
# functions to set dependency paths

pkg_config_path   = set()
cmake_prefix_path = set()

def use_system(dep):
    print_verbose(f'{dep}: {SYSTEM_ASSUMPTION}')

def use_pkg_config(dep, arg, subdir='lib/pkgconfig'):
    if(arg != SYSTEM_ASSUMPTION):
        path = os.path.realpath(arg) + '/' + subdir
        print_verbose(f'{dep}: using pkg-config files from {path}')
        pkg_config_path.add(path)
    else:
        use_system(dep)

def use_cmake(dep, path):
    if(arg != SYSTEM_ASSUMPTION):
        path = os.path.realpath(arg)
        print_verbose(f'{dep}: using cmake files from {path}')
        cmake_prefix_path.add(path)
    else:
        use_system(dep)

##################################################
# resolve dependencies

use_pkg_config('hipo', args.hipo)
use_pkg_config('fmt', args.fmt)

##################################################
# generate the build options

# INI options
def ini_string_arr(arr):
    contents = ','.join(map(lambda s: f'\'{s}\'', arr))
    return f'[{contents}]'
ini_opts = []
if(len(pkg_config_path) > 0):
    ini_opts.append(f'pkg_config_path = {ini_string_arr(pkg_config_path)}')
if(len(cmake_prefix_path) > 0):
    ini_opts.append(f'cmake_prefix_path = {ini_string_arr(cmake_prefix_path)}')

# CLI options
cli_opts = []
if(len(pkg_config_path) > 0):
    cli_opts.append(f'--pkg-config-path={",".join(pkg_config_path)}')
if(len(cmake_prefix_path) > 0):
    cli_opts.append(f'--cmake-prefix-path={",".join(cmake_prefix_path)}')

##################################################
# print

# header
if(len(pkg_config_path)==0 and len(cmake_prefix_path)==0):
    print_verbose(textwrap.dedent(f'''
    ==========================================================================================
    All of your dependencies are assumed to be in the system default locations.
    - If they are not, please run:
        {sys.argv[0]} --help
    - Otherwise, you do not need to set or modify any build options for dependency resolution.
    ==========================================================================================
    '''))
    exit(0)
else:
    print_verbose(textwrap.dedent('''
    ===============================================
    |  Here are the build options that you need:  |
    ===============================================
    '''))

print_verbose('>>> INI build options: add (or modify) the following lines to your build INI file:\n')
if(verbose or args.ini):
    for opt in ini_opts:
        print(opt)
print_verbose('\n')

print_verbose('>>> Alternatively, use these build options when running `meson setup`:\n')
if(verbose or args.cli):
    print(f'{" ".join(cli_opts)}')
print_verbose('\n')
