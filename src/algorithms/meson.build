algo_sources = [
  'AlgorithmFactory.cc',
  'AlgorithmSequence.cc',
  'clas12/EventBuilderFilter.cc',
  'clas12/LorentzTransformer.cc',
]

algo_lib = shared_library(
  'IguanaAlgorithms',
  algo_sources,
  include_directories: project_inc,
  dependencies:        project_deps,
  link_with:           services_lib,
  install:             true,
  install_rpath:       project_lib_rpath,
)
project_libs += algo_lib
