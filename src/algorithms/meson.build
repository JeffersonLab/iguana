algo_headers = [
  'clas12/event_builder_filter/EventBuilderFilter.h',
]

algo_sources = [
  'clas12/event_builder_filter/EventBuilderFilter.cc',
]

algo_lib = shared_library(
  'IguanaAlgorithms',
  algo_sources,
  include_directories: project_inc,
  dependencies:        services_dep,
  link_with:           services_lib,
  install:             true,
  install_dir:         project_lib_install_dir,
  install_rpath:       project_lib_rpath,
)

install_headers(algo_headers, subdir : meson.project_name())
