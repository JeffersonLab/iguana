project(
  'iguana',
  'cpp',
  version: '0.9.0',
  license: [ 'LGPLv3' ],
  license_files: [ 'LICENSE' ],
  meson_version: '>=1.2',
  default_options: {
    'cpp_std': 'c++17',
    'buildtype': 'release',
    'libdir': 'lib',
    'licensedir': 'share/licenses/iguana',
    'pkgconfig.relocatable': 'true',
    'force_fallback_for': ['rcdb'],
  },
)
project_description = 'Implementation Guardian of Analysis Algorithms'

# initialize binding languanges
add_languages('fortran', native: false, required: get_option('bind_fortran'))
use_chameleon = get_option('bind_fortran')

# warn that macOS is no longer tested
if host_machine.system() == 'darwin'
  warning('''host machine system is darwin:
  ==================================================================================
  It looks like you are using macOS.
  - Tests on macOS have been removed as of Iguana v0.9.0, in favor of testing
    in a Linux container.
  - For consistency and better support, using a Linux container is preferred.
  - Otherwise, please report any problems on macOS, if you choose to continue.
  ==================================================================================
  ''')
endif

# meson modules
pkg = import('pkgconfig')
fs = import('fs')

# build subprojects
rcdb_subproj = subproject('rcdb', required: get_option('z_require_rcdb'))

# resolve dependencies
fmt_dep = dependency(
  'fmt',
  version: '>=9.1.0',
  method: 'pkg-config',
)
yamlcpp_dep = dependency(
  'yaml-cpp',
  version: '>=0.7.0',
  method: 'pkg-config',
)
hipo_dep = dependency(
  'hipo4',
  version: '>=4.2.0',
  method: 'pkg-config',
)
ROOT_dep = dependency(
  'ROOT',
  version: '>=6.28.10',
  method: 'cmake',
  required: get_option('z_require_root'),
)
prog_ruby = find_program(
  'ruby',
  version: '>=3.0.0',
  required: use_chameleon,
)
rcdb_dep = dependency(
  'rcdb',
  fallback: ['rcdb', 'rcdb_dep'],
  required: get_option('z_require_rcdb'),
)
thread_dep = dependency(
  'threads',
  required: true, # FIXME: actually, it's only needed for multi-threading tests/examples
)

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
    '-lGraf',
    '-lPostscript',
  ]
  ROOT_dep_rpath = ROOT_libdir
  # preprocessor macro for turning on/off ROOT-dependent parts of the code; currently only used for Validator plot styles
  add_project_arguments('-DIGUANA_ROOT_FOUND', language: [ 'cpp' ])
endif

# general project vars
project_inc = include_directories('src')
project_libs = []
project_deps = declare_dependency(dependencies: dep_list) # do NOT include ROOT here
project_etcdir = get_option('sysconfdir') / meson.project_name()
project_test_env = environment()
project_pythondir = 'python'

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
  'suppressions=' + meson.project_source_root() / 'meson' / 'asan.supp',
)
project_test_env.set(
  'LSAN_OPTIONS',
  'suppressions=' + meson.project_source_root() / 'meson' / 'lsan.supp',
)

# set preprocessor macros
add_project_arguments(
  '-DIGUANA_ETCDIR="' + get_option('prefix') / project_etcdir + '"',
  language: [ 'cpp' ],
)

# start chameleon
if use_chameleon
  subdir('src/chameleon')
endif

# build and install shared libraries
subdir('src/iguana/services')
subdir('src/iguana/bankdefs')
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
pkg.generate(
  name: meson.project_name(),
  description: project_description,
  libraries: project_libs,
  requires: [ fmt_dep, yamlcpp_dep, hipo_dep ], # pkg-config dependencies only
)

# install environment setup files
if get_option('z_install_envfile')
  configure_file(
    input: 'meson' / 'this_iguana.sh.in',
    output: 'this_iguana.sh',
    install: true,
    install_dir: get_option('bindir'),
    configuration: {
      'ld_path':           host_machine.system() != 'darwin' ? 'LD_LIBRARY_PATH' : 'DYLD_LIBRARY_PATH',
      'python':            get_option('bind_python') ? 'true' : 'false',
      'root':              ROOT_dep.found() ? 'true' : 'false',
      'libdir':            get_option('libdir'),
      'includedir':        get_option('includedir'),
      'bindir':            get_option('bindir'),
      'etcdir':            project_etcdir,
      'pythondir':         project_pythondir,
      'dep_pkgconfigdirs': ':'.join(dep_pkgconfig_dirs),
      'dep_libdirs':       ':'.join(dep_lib_dirs),
      'dep_includedirs':   ':'.join(dep_include_dirs),
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
