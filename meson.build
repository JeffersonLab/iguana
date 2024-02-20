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
  static:  true,
  method:  'pkg-config',
  version: '>=4.0.1',
)
root_dep = dependency(
  'ROOT',
  method: 'cmake',
  version: run_command('meson' / 'minimum-version.sh', 'ROOT', check: true).stdout().strip(),
  required: false,
)

# list of dependencies
# FIXME: for users which use LD_LIBRARY_PATH, we should try to keep this list
# ordered such that the ones users are *least likely* to try to build
# themselves are listed last (see FIXME in meson/this_iguana.sh.in)
dep_list_full = [
  hipo_dep,
  fmt_dep,
  yamlcpp_dep,
]
if root_dep.found()
  dep_list_full += root_dep
else
  warning('ROOT is not found; code which depends on ROOT will not be included in this build.')
endif

# ROOT has some issues being included as a dependency, so make another list without it:
dep_list_full_NO_ROOT = []
if root_dep.found()
  foreach dep : dep_list_full
    if dep.name() != 'ROOT'
      dep_list_full_NO_ROOT += dep
    endif
  endforeach
else
  dep_list_full_NO_ROOT = dep_list_full
endif

# handle dependency-detection-method-dependent stuff (pkg-config vs. cmake):
# - copy the list of dependencies into dedicated lists: `dep_list_pkgconfig` and `dep_list_cmake`
# - make a list of dependency library and include directories, `dep_lib_paths` and `dep_inc_paths`
dep_list_pkgconfig = []
dep_list_cmake     = []
dep_lib_paths      = []
dep_inc_paths      = []
foreach dep : dep_list_full
  ### pkg-config deps
  if dep.type_name() == 'pkgconfig'
    dep_list_pkgconfig += dep
    libdirs = [ dep.get_variable(pkgconfig: 'libdir') ]
    incdirs = [ dep.get_variable(pkgconfig: 'includedir') ]
  ### cmake deps
  elif dep.type_name() == 'cmake'
    dep_list_cmake += dep
    libdirs = []
    foreach lib : dep.get_variable(cmake: 'PACKAGE_LIBRARIES').split(';')
      libdirs += run_command('dirname', lib, check: true).stdout().strip()
    endforeach
    incdirs = root_dep.get_variable(cmake: 'PACKAGE_INCLUDE_DIRS').split(';')
  ### error, if unknown
  else
    error('Cannot determine how dependency "' + dep.name() + '" was found')
  endif
  ### append to `dep_lib_paths` and `dep_inc_paths`
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

# general project vars
project_lib_rpath = '$ORIGIN'
project_inc       = include_directories('src')
project_libs      = []
project_deps      = declare_dependency(dependencies: dep_list_full_NO_ROOT)
project_etc       = get_option('sysconfdir') / meson.project_name()
project_pkg_vars  = [
  'dep_pkgconfigdirs=' + ':'.join(get_option('pkg_config_path')),
  'dep_libdirs='       + ':'.join(dep_lib_paths),
]

# executables' rpath
project_exe_rpath = []
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
  requires:    dep_list_pkgconfig,
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
