algo_sources = [
  'Algorithm.cc',
  'AlgorithmFactory.cc',
  'AlgorithmSequence.cc',
  'example/ExampleAlgorithm.cc',
  'clas12/EventBuilderFilter.cc',
  'clas12/LorentzTransformer.cc',
]

algo_public_headers = [
  'Algorithm.h',
  'AlgorithmBoilerplate.h',
  'AlgorithmSequence.h',
  'TypeDefs.h',
  'example/ExampleAlgorithm.h',
  'clas12/EventBuilderFilter.h',
  'clas12/LorentzTransformer.h',
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

install_headers(
  algo_public_headers,
  subdir:        meson.project_name() / 'algorithms',
  preserve_path: true,
)
