project(
  'iguana',
  'cpp',
  license: [ 'LGPLv3' ],
  license_files: [ 'LICENSE' ],
  meson_version: '>=1.2',
  default_options: {
    'cpp_std': 'c++17',
    'buildtype': 'release',
    'libdir': 'lib',
    'licensedir': 'share/licenses/iguana',
    'pkgconfig.relocatable': 'true',
  },
  version: run_command(
    meson.project_source_root() / 'meson' / 'detect-version.sh',
    meson.project_source_root(),
    check: true,
  ).stdout().strip(),
)
project_description = 'Implementation Guardian of Analysis Algorithms'

# initialize binding languanges
add_languages('fortran', native: false, required: get_option('bind_fortran'))
use_chameleon = get_option('bind_fortran')

# meson modules
pkg = import('pkgconfig')
fs = import('fs')

# build subprojects
rcdb_subproj = subproject(
  'rcdb',
  default_options: {
    'home': run_command('meson' / 'get-env.py', 'RCDB_HOME', check: false).stdout().strip(),
  }
)

# resolve dependencies
prog_minver = find_program('meson' / 'minimum-version.sh')
# NOTE: those that are typically installed by package managers should use `meson/minimum-version.sh`
fmt_dep = dependency(
  'fmt',
  method: 'pkg-config',
  version: '>=' + run_command(prog_minver, 'fmt', check: true).stdout().strip(),
)
yamlcpp_dep = dependency(
  'yaml-cpp',
  method: 'pkg-config',
  version: '>=' + run_command(prog_minver, 'yaml-cpp', check: true).stdout().strip(),
)
hipo_dep = dependency(
  'hipo4',
  method: 'pkg-config',
  version: '>=4.1.0'
)
ROOT_dep = dependency(
  'ROOT',
  required: get_option('z_require_root'),
  method: 'cmake',
  version: '>=' + run_command(prog_minver, 'ROOT', check: true).stdout().strip(),
)
prog_ruby = find_program(
  'ruby',
  version: '>=' + run_command(prog_minver, 'ruby', check: true).stdout().strip(),
  required: use_chameleon,
)
rcdb_dep = dependency('rcdb', fallback: ['rcdb', 'rcdb_dep'], required: false)

# list of dependencies
# FIXME: for users which use LD_LIBRARY_PATH, we should try to keep this list
# ordered such that the ones users are *least likely* to try to build
# themselves are listed last (see FIXME in meson/this_iguana.sh.in)
# NOTE: omit ROOT (handled differently, and most ROOT users already have it in their environment)
dep_list = []
foreach dep : [ hipo_dep, fmt_dep, yamlcpp_dep, rcdb_dep ]
  if dep.found()
    dep_list += dep
  endif
endforeach

# pkgconfig configuration: make a list of dependency library and include directories
dep_lib_dirs = []
dep_include_dirs = []
dep_pkgconfig_dirs = []
foreach dep : dep_list

  # get library and include dirs
  if dep.type_name() == 'pkgconfig'
    libdirs = [ dep.get_variable(pkgconfig: 'libdir') ]
    incdirs = [ dep.get_variable(pkgconfig: 'includedir') ]
  elif dep.type_name() == 'cmake'
    libdirs = []
    foreach lib : dep.get_variable(cmake: 'PACKAGE_LIBRARIES').split(';')
      libdirs += run_command('dirname', lib, check: true).stdout().strip()
    endforeach
    incdirs = ROOT_dep.get_variable(cmake: 'PACKAGE_INCLUDE_DIRS').split(';')
  else
    name = dep.get_variable(internal: 'name', default_value: dep.name())
    if name == 'rcdb'
      incdirs = [ dep.get_variable(internal: 'includedir') ]
    else
      warning(f'Unknown dependency "@name@"')
      continue
    endif
  endif

  # append to `dep_*_dirs` arrays, uniquely
  foreach libdir : libdirs
    if not dep_lib_dirs.contains(libdir)
      dep_lib_dirs += libdir
    endif
    if dep.type_name() == 'pkgconfig'
      pkgconfigdir = libdir / 'pkgconfig'
      if not dep_pkgconfig_dirs.contains(pkgconfigdir)
        dep_pkgconfig_dirs += pkgconfigdir
      endif
    endif
  endforeach
  foreach incdir : incdirs
    if not dep_include_dirs.contains(incdir)
      dep_include_dirs += incdir
    endif
  endforeach
endforeach

# handle HIPO dataframes
hipo_dep_dataframes_found = hipo_dep.get_variable(
  pkgconfig: 'with_dataframes',
  default_value: 'false',
).to_lower() == 'true'

# handle ROOT
ROOT_dep_inc_dirs = []
ROOT_dep_link_args = []
ROOT_dep_link_args_for_validators = []
ROOT_dep_rpath = ''
if ROOT_dep.found()
  ROOT_dep_inc_dirs += include_directories(
    run_command('root-config', '--incdir', check: true).stdout().strip(),
  )
  ROOT_libdir = run_command('root-config', '--libdir', check: true).stdout().strip()
  # ROOT libraries that we need (safer than `root-config --libs`)
  ROOT_dep_link_args += [
    '-L' + ROOT_libdir,
    '-lCore',
    '-lGenVector',
    '-lROOTDataFrame',
    '-lROOTVecOps',
    '-lTreePlayer',
  ]
  # additional ROOT libraries for validators (namely, graphics libraries)
  ROOT_dep_link_args_for_validators = [
    '-L' + ROOT_libdir,
    '-lRIO',
    '-lHist',
    '-lGpad',
  ]
  ROOT_dep_rpath = ROOT_libdir
endif

# general project vars
project_inc = include_directories('src')
project_libs = []
project_deps = declare_dependency(dependencies: dep_list) # do NOT include ROOT here
project_etc = get_option('sysconfdir') / meson.project_name()
project_test_env = environment()
project_pkg_vars = [
  'bindir='            + '${prefix}' / get_option('bindir'),
  'dep_pkgconfigdirs=' + ':'.join(dep_pkgconfig_dirs),
  'dep_libdirs='       + ':'.join(dep_lib_dirs),
  'dep_includedirs='   + ':'.join(dep_include_dirs),
]

# sanitizer settings
project_test_env.set(
  'UBSAN_OPTIONS',
  'halt_on_error=1',
  'abort_on_error=1',
  'print_summary=1',
  'print_stacktrace=1',
  'suppressions=' + meson.project_source_root() / 'meson' / 'ubsan.supp',
)
project_test_env.set(
  'ASAN_OPTIONS',
  'halt_on_error=1',
  'abort_on_error=1',
  'print_summary=1',
)
project_test_env.set(
  'LSAN_OPTIONS',
  'suppressions=' + meson.project_source_root() / 'meson' / 'lsan.supp',
)

# set preprocessor macros
add_project_arguments(
  '-DIGUANA_ETCDIR="' + get_option('prefix') / project_etc + '"',
  language: [ 'cpp' ],
)

# start chameleon
if use_chameleon
  subdir('src/chameleon')
endif

# build and install shared libraries
subdir('src/iguana/services')
subdir('src/iguana/algorithms')
subdir('src/iguana/tests')

# build and install bindings
if use_chameleon
  subdir('src/iguana/bindings')
endif
if get_option('bind_python')
  subdir('bind/python')
endif

# build examples
if get_option('install_examples')
  subdir('examples')
endif

# generate documentation
if get_option('install_documentation')
  subdir('doc/gen')
endif

# generate pkg-config file
project_pkg_vars_nonempty = []
foreach var : project_pkg_vars
  if not var.endswith('=') # remove empty variable values
    project_pkg_vars_nonempty += var
  endif
endforeach
pkg.generate(
  name: meson.project_name(),
  description: project_description,
  libraries: project_libs,
  requires: [ fmt_dep, yamlcpp_dep, hipo_dep ], # pkg-config dependencies only
  variables: project_pkg_vars_nonempty,
)

# install environment setup files
if get_option('z_install_envfile')
  configure_file(
    input: 'meson' / 'this_iguana.sh.in',
    output: 'this_iguana.sh',
    install: true,
    install_dir: get_option('bindir'),
    configuration: {
      'ld_path': host_machine.system() != 'darwin' ? 'LD_LIBRARY_PATH' : 'DYLD_LIBRARY_PATH',
      'python': get_option('bind_python') ? 'true' : 'false',
      'libdir': get_option('libdir'),
      'root': ROOT_dep.found() ? 'true' : 'false',
      'etcdir': project_etc,
    },
  )
  foreach shell : [ 'csh', 'tcsh' ]
    configure_file(
      input: 'meson' / 'this_iguana.csh.in',
      output: 'this_iguana.' + shell,
      install: true,
      install_dir: get_option('bindir'),
      configuration: {'shell': shell},
    )
  endforeach
endif
