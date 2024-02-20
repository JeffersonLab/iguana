# algorithm source files
algo_sources = [
  'Algorithm.cc',
  'AlgorithmFactory.cc',
  'AlgorithmSequence.cc',
  'example/ExampleAlgorithm.cc',
  'clas12/EventBuilderFilter.cc',
  'clas12/ZVertexFilter.cc',
  'clas12/LorentzTransformer.cc',
]

# algorithm headers
algo_public_headers = [
  'Algorithm.h',
  'AlgorithmBoilerplate.h',
  'AlgorithmSequence.h',
  'TypeDefs.h',
  'example/ExampleAlgorithm.h',
  'clas12/EventBuilderFilter.h',
  'clas12/ZVertexFilter.h',
  'clas12/LorentzTransformer.h',
]

algo_config_files = [
  'clas12/ZVertexFilter.yaml',
]

# algorithm unique names and required banks, for those we want to test automatically
algos_and_banks_for_unit_testing = {
  'example::ExampleAlgorithm':  ['REC::Particle'],
  'clas12::EventBuilderFilter': ['REC::Particle'],
  'clas12::ZVertexFilter':      ['REC::Particle'],
  'clas12::LorentzTransformer': ['REC::Particle'],
}

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

foreach algo_config_file : algo_config_files
  install_data(
    algo_config_file,
    install_dir:   project_etc,
    preserve_path: true,
  )
endforeach
