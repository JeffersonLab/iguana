algo_headers = [
  'clas12/event_builder_filter/EventBuilderFilter.h',
  'clas12/lorentz_transformer/LorentzTransformer.h',
]

algo_sources = [
  'clas12/event_builder_filter/EventBuilderFilter.cc',
  'clas12/lorentz_transformer/LorentzTransformer.cc',
]

algo_lib = shared_library(
  'IguanaAlgorithms',
  algo_sources,
  include_directories: project_inc,
  dependencies:        services_dep,
  link_with:           services_lib,
  install:             true,
  install_rpath:       project_lib_rpath,
)

install_headers(algo_headers, subdir : meson.project_name())
