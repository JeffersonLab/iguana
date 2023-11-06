algo_headers = [
  'FiducialCuts.h',
]

algo_sources = [
  'FiducialCuts.cc',
]

algo_lib = shared_library(
  'IguanaAlgorithms',
  algo_sources,
  include_directories: project_inc,
  dependencies:        services_dep,
  link_with:           services_lib,
  install_rpath:       project_lib_rpath,
  install:             true,
)

install_headers(algo_headers, subdir : meson.project_name())
