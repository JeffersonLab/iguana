project(
  'iguana',
  'cpp',
  license:         'LGPLv3',
  default_options: [ 'cpp_std=c++17' ],
  meson_version:   '>=1.1',
  version: run_command(
    meson.project_source_root() / 'meson' / 'detect-version.sh',
    meson.project_source_root(),
    check: true
  ).stdout().strip()
)
project_description = 'Implementation Guardian of Analysis Algorithms'

# resolve dependencies
fmt_dep  = dependency('fmt', version: '>=9.1.0')
hipo_dep = dependency('hipo4', version: '>=1.3', method: 'cmake')

# dependency library paths
dep_lib_paths = []
foreach path : get_option('cmake_prefix_path')
  dep_lib_paths += path / 'lib'
endforeach
foreach path : get_option('pkg_config_path')
  dep_lib_paths += path / '..'
endforeach

# general project vars
project_lib_rpath = '$ORIGIN'
project_inc       = include_directories('src')
project_libs      = []
project_deps      = declare_dependency(dependencies: [ fmt_dep, hipo_dep ])
project_pkg_vars  = [ 'dep_lib_paths=' + ':'.join(dep_lib_paths) ]

# build and install shared libraries
subdir('src/iguana/services')
subdir('src/iguana/algorithms')

# build bindings
if(get_option('bind_python'))
  subdir('bind/python')
endif

# generate pkg-config file
pkg = import('pkgconfig')
pkg.generate(
  name:        meson.project_name(),
  description: project_description,
  libraries:   project_libs,
  requires:    [ fmt_dep ],
  variables:   project_pkg_vars,
)

# build examples
if(get_option('examples'))
  subdir('examples')
endif

# generate documentation
if(get_option('documentation'))
  doxygen = find_program('doxygen', required: false)
  if(doxygen.found())
    message('Generating documentation...')
    run_command('doxygen', meson.project_source_root() / 'doc' / 'Doxyfile', check: true)
    message('...documentation generated.')
    install_subdir('doc/api', install_dir: 'doc')
  else
    warning('Cannot generate documentation since `doxygen` is not found')
  endif
endif

# install environment setup file
install_data(
  'meson' / 'this_iguana.sh.in',
  install_dir: get_option('bindir'),
  rename:      'this_iguana.sh'
)
