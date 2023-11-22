services_headers = [
  'TypeDefs.h',
  'Logger.h',
  'Algorithm.h',
]

services_sources = [
  'Logger.cc',
  'Algorithm.cc',
]

services_lib = shared_library(
  'IguanaServices',
  services_sources,
  include_directories: project_inc,
  dependencies:        [ fmt_dep, hipo_dep ],
  install:             true,
  install_dir:         project_lib_install_dir,
  install_rpath:       project_lib_rpath,
)

services_dep = declare_dependency(
  dependencies: [ fmt_dep, hipo_dep ]
)

install_headers(services_headers, subdir : meson.project_name())
