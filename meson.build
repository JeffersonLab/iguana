project(
  'iguana',
  'cpp',
  license:         'LGPLv3',
  default_options: {
    'cpp_std':               'c++17',
    'buildtype':             'release',
    'libdir':                'lib',
    'pkgconfig.relocatable': 'true',
  },
  meson_version:   '>=1.2',
  version: run_command(
    meson.project_source_root() / 'meson' / 'detect-version.sh',
    meson.project_source_root(),
    check: true
  ).stdout().strip()
)
project_description = 'Implementation Guardian of Analysis Algorithms'

# resolve dependencies
fmt_dep  = dependency('fmt',   version: '>=9.1.0', method: 'pkg-config')
hipo_dep = dependency('hipo4', version: '>=4.0.1', method: 'pkg-config')
yamlcpp_dep = dependency('yaml-cpp', version: '>=0.8.0', method: 'pkg-config')

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
project_pkg_vars  = [
  'dep_pkgconfigdirs=' + ':'.join(get_option('pkg_config_path')),
  'dep_libdirs='       + ':'.join(dep_lib_paths),
]

# build and install shared libraries
subdir('src/iguana/services')
subdir('src/iguana/algorithms')

# build bindings
if get_option('bind_python')
  subdir('bind/python')
endif

# generate pkg-config file
pkg = import('pkgconfig')
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
