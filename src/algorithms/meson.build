algo_headers = {
  'algo': [
    'AlgorithmFactory.h',
    'AlgorithmSequence.h',
  ],
  'clas12': [
    'clas12/event_builder_filter/EventBuilderFilter.h',
    'clas12/lorentz_transformer/LorentzTransformer.h',
  ],
}

algo_sources = {
  'algo': [
    'AlgorithmFactory.cc',
    'AlgorithmSequence.cc',
  ],
  'clas12': [
    'clas12/event_builder_filter/EventBuilderFilter.cc',
    'clas12/lorentz_transformer/LorentzTransformer.cc',
  ],
}

algo_sources_flattened = []
foreach key : algo_sources.keys()
  algo_sources_flattened += algo_sources[key]
endforeach

algo_lib = shared_library(
  'IguanaAlgorithms',
  algo_sources_flattened,
  include_directories: project_inc,
  dependencies:        services_dep,
  link_with:           services_lib,
  install:             true,
  install_rpath:       project_lib_rpath,
)

foreach key : algo_headers.keys()
  dir = key == 'algo' ? '' : '/'+key
  install_headers(algo_headers[key], subdir: meson.project_name() + dir)
endforeach
