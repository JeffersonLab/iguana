# dictionary for info about each algorithm
#
#  example:
#  ========
#   ALGORITHM_FULL_NAME: {
#     'sources':      LIST_OF_SOURCE FILES,
#     'headers':      LIST_OF_HEADER_FILES,
#     'configs':      LIST_OF_CONFIG_FILES,
#     'unit_test':    DICTIONARY_FOR_UNIT_TESTING_ARGS,  # if excluded, unit tests won't run for this algorithm
#     'needs_ROOT':   BOOLEAN,  # whether this algorithm needs ROOT or not (default=false)
#   }
#
algo_dict = {
  'main': {
    'sources': [ 'Algorithm.cc', 'AlgorithmFactory.cc', 'AlgorithmSequence.cc' ],
    'headers': [ 'Algorithm.h', 'AlgorithmBoilerplate.h', 'TypeDefs.h', 'AlgorithmSequence.h' ],
  },
  'example::ExampleAlgorithm': {
    'sources':      [ 'example/ExampleAlgorithm.cc' ],
    'headers':      [ 'example/ExampleAlgorithm.h' ],
    'configs':      [ 'example/ExampleAlgorithm.yaml' ],
    'unit_test':    { 'banks': ['REC::Particle'] },
  },
  'clas12::EventBuilderFilter': {
    'sources':      [ 'clas12/EventBuilderFilter.cc' ],
    'headers':      [ 'clas12/EventBuilderFilter.h' ],
    'configs':      [ 'clas12/EventBuilderFilter.yaml' ],
    'unit_test':    { 'banks': ['REC::Particle'] },
  },
  'clas12::ZVertexFilter': {
    'sources':      [ 'clas12/ZVertexFilter.cc' ],
    'headers':      [ 'clas12/ZVertexFilter.h' ],
    'configs':      [ 'clas12/ZVertexFilter.yaml' ],
    'unit_test':    { 'banks': ['REC::Particle'] },
  },
  'clas12::SectorFinder': {
    'sources':      [ 'clas12/SectorFinder.cc' ],
    'headers':      [ 'clas12/SectorFinder.h' ],
    'configs':      [ 'clas12/SectorFinder.yaml' ],
    'unit_test':    { 'banks': ['REC::Particle','REC::Calorimeter','REC::Track','REC::SCintillator','REC::Cherenkov'] },
  },
  'clas12::LorentzTransformer': {
    'sources':      [ 'clas12/LorentzTransformer.cc' ],
    'headers':      [ 'clas12/LorentzTransformer.h' ],
    'configs':      [ 'clas12/LorentzTransformer.yaml' ],
    'unit_test':    { 'banks': ['REC::Particle'] },
    'needs_ROOT':   true,
  },
  'clas12::MomentumCorrection': {
    'sources':      [ 'clas12/MomentumCorrection.cc' ],
    'headers':      [ 'clas12/MomentumCorrection.h' ],
    'unit_test':    { 'banks': ['REC::Particle', 'RUN::config'] },
  },
}

# make lists of objects to build; inclusion depends on whether ROOT is needed or not, and if we have ROOT
algo_sources = []
algo_headers = []
algo_configs = []
foreach name, info : algo_dict
  needs_ROOT = info.get('needs_ROOT', false)
  if (needs_ROOT and ROOT_dep.found()) or not needs_ROOT
    algo_sources += info.get('sources', [])
    algo_headers += info.get('headers', [])
    algo_configs += info.get('configs', [])
  else
    warning('Excluding algorithm "' + name + '", which depends on ROOT')
  endif
endforeach

# build and install
algo_lib = shared_library(
  'IguanaAlgorithms',
  algo_sources,
  include_directories: [ project_inc ] + ROOT_dep_inc_dirs,
  dependencies:        project_deps,
  link_with:           services_lib,
  link_args:           ROOT_dep_link_args,
  install:             true,
  install_rpath:       project_lib_rpath,
)
project_libs += algo_lib

install_headers(
  algo_headers,
  subdir:        meson.project_name() / 'algorithms',
  preserve_path: true,
)

foreach algo_config : algo_configs
  install_data(
    algo_config,
    install_dir:   project_etc / 'algorithms',
    preserve_path: true,
  )
endforeach
