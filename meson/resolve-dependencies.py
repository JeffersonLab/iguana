#!/usr/bin/env python3

from configparser import ConfigParser
import argparse, os, sys, textwrap

# constants
SYSTEM_ASSUMPTION = 'assume system installation'
NOT_USED          = 'not used'

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
parser_deps = parser.add_argument_group('dependency installation paths')
parser_deps.add_argument('--hipo', default=SYSTEM_ASSUMPTION, type=str, help='path to `hipo` installation')
parser_deps.add_argument('--fmt', default=SYSTEM_ASSUMPTION, type=str, help='path to `fmt` installation')
parser_output = parser.add_argument_group('output control')
parser_output.add_argument('--cli', default=False, action=argparse.BooleanOptionalAction, help='only print the `meson` CLI options, and nothing else')
parser_output.add_argument('--ini', default=NOT_USED, type=str, help='if set, generate an INI file (meson native file) with this name; you may then use it with `meson setup --native-file=_____`')
args = parser.parse_args()

# verbosity
verbose = not args.cli
def print_verbose(message):
    if(verbose):
        print(message)

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

# resolve dependencies #########################
use_pkg_config('hipo', args.hipo)
use_pkg_config('fmt', args.fmt)
################################################


# generate a native file
if(args.ini!=NOT_USED):
    def ini_string_arr(arr):
        contents = ','.join(map(lambda s: f'\'{s}\'', arr))
        return f'[{contents}]'
    ini_config = ConfigParser(allow_no_value=True)
    ini_config.add_section('built-in options')
    if(len(cmake_prefix_path) > 0):
        ini_config.set('built-in options', 'cmake_prefix_path', ini_string_arr(cmake_prefix_path))
    if(len(pkg_config_path) > 0):
        ini_config.set('built-in options', 'pkg_config_path', ini_string_arr(pkg_config_path))
    with open(args.ini, 'w') as fp:
        ini_config.write(fp)

# generate CLI options
if(verbose or args.cli):
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
        cli_opts = []
        if(len(pkg_config_path) > 0):
            cli_opts.append(f'--pkg-config-path={",".join(pkg_config_path)}')
        if(len(cmake_prefix_path) > 0):
            cli_opts.append(f'--cmake-prefix-path={",".join(cmake_prefix_path)}')
        if(args.ini==NOT_USED):
            print(f'{" ".join(cli_opts)}')
        else:
            print(f'--native-file={args.ini}')
        print_verbose('\n')
