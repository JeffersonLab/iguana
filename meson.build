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
project_inc       = include_directories('include')
project_libs      = []
project_deps      = declare_dependency(dependencies: [ fmt_dep, hipo_dep ])

# build and install shared libraries
subdir('src/services')
subdir('src/algorithms')
install_subdir('include/iguana', install_dir: get_option('includedir'))

# generate pkg-config file for shared libraries
pkg = import('pkgconfig')
pkg.generate(
  name:        meson.project_name(),
  description: project_description,
  libraries:   project_libs,
)

# build examples
if(get_option('examples'))
  subdir('examples')
endif
