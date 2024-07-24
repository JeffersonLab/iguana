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
parser_deps.add_argument('--yaml', default=SYSTEM_ASSUMPTION, type=str, help='path to `yaml-cpp` installation')
parser_deps.add_argument('--root', default=SYSTEM_ASSUMPTION, type=str, help='path to `ROOT` installation')
parser_deps.add_argument('--rcdb', default='$RCDB_HOME', type=str, help='path to `RCDB` installation')
parser_output = parser.add_argument_group('output control')
parser_output.add_argument('--verbose', default=False, action=argparse.BooleanOptionalAction, help='verbose output')
args = parser.parse_args()

# verbosity
def print_verbose(message):
    if(args.verbose):
        print(message)

# functions to set dependency paths
pkg_config_path   = set()
cmake_prefix_path = set()
extra_args        = []
def use_system(dep):
    print_verbose(f'{dep}: {SYSTEM_ASSUMPTION}')
def use_pkg_config(dep, pc_file, arg):
    if(arg != SYSTEM_ASSUMPTION):
        prefix = os.path.realpath(arg)
        pc_path = ''
        for root, dirs, files in os.walk(prefix):
            if pc_file in files:
                # be sure to choose the INSTALLED .pc file; FIXME: may not work for all dependencies, but so far this is okay
                if root.split('/')[-1] == 'pkgconfig':
                    pc_path = root
                    break
        if pc_path == '':
            print(f'ERROR: cannot find "{pc_file}" in any subdirectory of {arg}', file=sys.stderr)
            exit(1)
        print_verbose(f'{dep}: using pkg-config files from {pc_path}')
        pkg_config_path.add(pc_path)
    else:
        use_system(dep)
def use_cmake(dep, arg):
    if(arg != SYSTEM_ASSUMPTION):
        path = os.path.realpath(arg)
        print_verbose(f'{dep}: using cmake files from {path}')
        cmake_prefix_path.add(path)
    else:
        use_system(dep)
def use_env_var(dep, build_var_name, user_val, env_var_name):
    if(user_val == f'${env_var_name}'):
        if env_var_name in os.environ:
            print_verbose(f'{dep}: using environment variable "{env_var_name}" for build variable "{build_var_name}"')
            extra_args.append([build_var_name, os.environ[env_var_name]])
        else:
            print(f'{dep}: you did not specify where {dep} is found, and the fallback environment variable "${env_var_name}" is not set; {dep} will be ignored', file=sys.stderr)
    else:
        print_verbose(f'{dep}: using user value "{user_val}" for build variable "{build_var_name}"')
        extra_args.append([build_var_name, user_val])

# resolve dependencies #########################
use_pkg_config('hipo', 'hipo4.pc',    args.hipo)
use_pkg_config('fmt',  'fmt.pc',      args.fmt)
use_pkg_config('yaml', 'yaml-cpp.pc', args.yaml)
use_cmake('ROOT', args.root)
use_env_var('rcdb', 'rcdb:home', args.rcdb, 'RCDB_HOME')
################################################


# generate CLI options
if(len(pkg_config_path)==0 and len(cmake_prefix_path)==0 and len(extra_args)==0):
    print_verbose(textwrap.dedent(f'''
    ==========================================================================================
    All of your dependencies are assumed to be in the system default locations.
    - If they are not, please run:
        {sys.argv[0]} --help
    - Otherwise, you do not need to set or modify any build options for dependency resolution.
    ==========================================================================================
    '''))
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
    if(len(extra_args) > 0):
        for extra_arg in extra_args:
            cli_opts.append(f'-D{extra_arg[0]}={extra_arg[1]}')
    print(f'{" ".join(cli_opts)}')
    print_verbose('\n')
