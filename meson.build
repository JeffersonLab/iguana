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

# list of dependencies
# FIXME: for users which use LD_LIBRARY_PATH, we should try to keep this list
# ordered such that the ones users are *least likely* to try to build
# themselves are listed last (see FIXME in meson/this_iguana.sh.in)
full_dep_list = [
  hipo_dep,
  fmt_dep,
  yamlcpp_dep,
]

# dependency libdirs
dep_lib_paths = []
foreach dep : full_dep_list
  dep_lib_paths += dep.get_variable(pkgconfig: 'libdir')
endforeach

# general project vars
project_lib_rpath = '$ORIGIN'
project_inc       = include_directories('src')
project_libs      = []
project_deps      = declare_dependency(dependencies: full_dep_list)
project_etc       = get_option('sysconfdir') / meson.project_name()
project_pkg_vars  = [
  'dep_pkgconfigdirs=' + ':'.join(get_option('pkg_config_path')),
  'dep_libdirs='       + ':'.join(dep_lib_paths),
]

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
  requires:    full_dep_list,
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
  },
)
