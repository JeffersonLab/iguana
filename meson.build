project(
  'iguana',
  'cpp',
  license: 'LGPLv3',
  meson_version: '>=1.2',
  default_options: {
    'cpp_std':               'c++17',
    'buildtype':             'release',
    'libdir':                'lib',
    'pkgconfig.relocatable': 'true',
  },
  version: run_command(
    meson.project_source_root() / 'meson' / 'detect-version.sh',
    meson.project_source_root(),
    check: true
  ).stdout().strip()
)
project_description = 'Implementation Guardian of Analysis Algorithms'

# meson modules
pkg = import('pkgconfig')
fs  = import('fs')

# resolve dependencies
# NOTE: those that are typically installed by package managers should use `meson/minimum-version.sh`
fmt_dep = dependency(
  'fmt',
  method:  'pkg-config',
  version: run_command('meson' / 'minimum-version.sh', 'fmt', check: true).stdout().strip()
)
yamlcpp_dep = dependency(
  'yaml-cpp',
  method:  'pkg-config',
  version: run_command('meson' / 'minimum-version.sh', 'yaml-cpp', check: true).stdout().strip()
)
hipo_dep = dependency(
  'hipo4',
  method:  'pkg-config',
  version: '>=4.0.1',
)
ROOT_dep = dependency(
  'ROOT',
  required: get_option('require_ROOT'),
  method:   'cmake',
  version: run_command('meson' / 'minimum-version.sh', 'ROOT', check: true).stdout().strip()
)

# list of dependencies
# FIXME: for users which use LD_LIBRARY_PATH, we should try to keep this list
# ordered such that the ones users are *least likely* to try to build
# themselves are listed last (see FIXME in meson/this_iguana.sh.in)
dep_list = []
foreach dep : [ hipo_dep, fmt_dep, yamlcpp_dep, ROOT_dep ]
  if dep.found()
    dep_list += dep
  endif
endforeach

# make a list of dependency library and include directories
dep_lib_paths = []
dep_inc_paths = []
foreach dep : dep_list
  ### handle pkg-config deps
  if dep.type_name() == 'pkgconfig'
    libdirs = [ dep.get_variable(pkgconfig: 'libdir') ]
    incdirs = [ dep.get_variable(pkgconfig: 'includedir') ]
  ### handle cmake deps
  elif dep.type_name() == 'cmake'
    libdirs = []
    foreach lib : dep.get_variable(cmake: 'PACKAGE_LIBRARIES').split(';')
      libdirs += run_command('dirname', lib, check: true).stdout().strip()
    endforeach
    incdirs = ROOT_dep.get_variable(cmake: 'PACKAGE_INCLUDE_DIRS').split(';')
  ### error, if unknown
  else
    error('Cannot determine how dependency "' + dep.name() + '" was found')
  endif
  ### append to `dep_lib_paths` and `dep_inc_paths`, omitting duplicates
  foreach libdir : libdirs
    if not dep_lib_paths.contains(libdir)
      dep_lib_paths += libdir
    endif
  endforeach
  foreach incdir : incdirs
    if not dep_inc_paths.contains(incdir)
      dep_inc_paths += incdir
    endif
  endforeach
endforeach
message('Dependency library dirs = [', ', '.join(dep_lib_paths), ']')
message('Dependency include dirs = [', ', '.join(dep_inc_paths), ']')

# handle ROOT
ROOT_dep_inc_dirs  = []
ROOT_dep_link_args = []
ROOT_dep_link_args_for_validators = []
ROOT_dep_rpath = ''
if ROOT_dep.found()
  ROOT_dep_inc_dirs += include_directories(run_command('root-config', '--incdir', check: true).stdout().strip())
  ROOT_libdir = run_command('root-config', '--libdir', check: true).stdout().strip()
  ROOT_dep_link_args += [
    # ROOT libraries that we need (safer than `root-config --libs`)
    '-L' + ROOT_libdir,
    '-lCore',
    '-lGenVector',
  ]
  ROOT_dep_link_args_for_validators = [
    # additional ROOT libraries for validators (namely, graphics libraries)
    '-L' + ROOT_libdir,
    '-lRIO',
    '-lHist',
    '-lGpad',
  ]
  ROOT_dep_rpath = ROOT_libdir
endif

# general project vars
project_lib_rpath = '$ORIGIN'
project_inc       = include_directories('src')
project_libs      = []
project_deps      = declare_dependency(dependencies: [ fmt_dep, yamlcpp_dep, hipo_dep ] ) # do NOT include ROOT here
project_etc       = get_option('sysconfdir') / meson.project_name()
project_test_env  = environment()
project_pkg_vars  = [
  'dep_pkgconfigdirs=' + ':'.join(get_option('pkg_config_path')),
  'dep_libdirs='       + ':'.join(dep_lib_paths),
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

# executables' rpath
project_exe_rpath = [
  hipo_dep.get_variable(pkgconfig: 'libdir')
]
if host_machine.system() != 'darwin'
  # FIXME(darwin): not sure how to set multiple rpaths on darwin executables,
  # aside from running `install_name_tool -add_rpath` post-installation;
  # luckily, darwin-built executables don't need the `iguana` library path
  # explictly included in the rpath, so this `if` block just keeps
  # `project_exe_rpath` minimal. See https://github.com/mesonbuild/meson/issues/5760
  project_exe_rpath += '$ORIGIN' / '..' / get_option('libdir')
endif

# set preprocessor macros
add_project_arguments(
  '-DIGUANA_ETC="' + get_option('prefix') / project_etc + '"',
  language: ['cpp'],
)

# build and install shared libraries
subdir('src/iguana/services')
subdir('src/iguana/algorithms')
subdir('src/iguana/tests')

# build bindings
if get_option('bind_python')
  subdir('bind/python')
endif

# generate pkg-config file
pkg.generate(
  name:        meson.project_name(),
  description: project_description,
  libraries:   project_libs,
  requires:    [ fmt_dep, yamlcpp_dep, hipo_dep ], # pkg-config dependencies only
  variables:   project_pkg_vars,
)

# build examples
if get_option('examples')
  subdir('examples')
endif

# generate documentation
if get_option('documentation')
  doxygen = find_program('doxygen', required: false)
  if doxygen.found()
    message('Generating documentation...')
    run_command('doxygen', meson.project_source_root() / 'doc' / 'Doxyfile', check: true)
    message('...documentation generated.')
    install_subdir('doc/api', install_dir: 'doc')
  else
    warning('Cannot generate documentation since `doxygen` is not found')
  endif
endif

# install environment setup file
configure_file(
  input:         'meson' / 'this_iguana.sh.in',
  output:        'this_iguana.sh',
  install:       true,
  install_dir:   get_option('bindir'),
  configuration: {
    'ld_path': host_machine.system() != 'darwin' ? 'LD_LIBRARY_PATH' : 'DYLD_LIBRARY_PATH',
    'python':  get_option('bind_python') ? 'true' : 'false',
    'libdir':  get_option('libdir'),
  },
)
