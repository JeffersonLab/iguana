project(
  'iguana',
  'cpp',
  version:         '0.0.0',
  license:         'LGPLv3',
  default_options: [ 'cpp_std=c++17' ],
  meson_version:   '>=1.1',
)
project_description = 'Implementation Guardian of Analysis Algorithms'

# resolve dependencies
fmt_dep  = dependency('fmt', version: '>=9.1.0')
hipo_dep = dependency('hipo4', version: '>=1.3', method: 'cmake')

# general project vars
project_lib_rpath = '$ORIGIN'
project_inc       = include_directories('src')
project_libs      = []
project_deps      = declare_dependency(dependencies: [ fmt_dep, hipo_dep ])

# build and install shared libraries
subdir('src/iguana/services')
subdir('src/iguana/algorithms')

# generate pkg-config file for shared libraries
pkg = import('pkgconfig')
pkg.generate(
  name:        meson.project_name(),
  description: project_description,
  libraries:   project_libs,
  requires:    [ fmt_dep ],
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
