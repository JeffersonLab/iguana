services_sources = [
  'Logger.cc',
  'Object.cc',
  'Algorithm.cc',
]

services_lib = shared_library(
  'IguanaServices',
  services_sources,
  include_directories: project_inc,
  dependencies:        project_deps,
  install:             true,
  install_rpath:       project_lib_rpath,
)
project_libs += services_lib
