# dictionary for info about each algorithm
#
#  example:
#  ========
#   ALGORITHM_FULL_NAME: {
#     'algorithm': {
#       'sources':      LIST_OF_ALGORITHM_SOURCE FILES,
#       'headers':      LIST_OF_ALGORITHM_HEADER_FILES,
#       'needs_ROOT':   BOOLEAN,  # whether this algorithm needs ROOT or not (default=false)
#     },
#     'validator': {    # if excluded, a validator test will not be built for this algorithm
#       'sources':      LIST_OF_VALIDATOR_SOURCE FILES,
#       'headers':      LIST_OF_VALIDATOR_HEADER_FILES,
#       'needs_ROOT':   BOOLEAN,  # whether this validator needs ROOT or not (default=false)
#     },
#     'configs':        LIST_OF_ALGORITHM_CONFIG_FILES,
#     'test_args':      DICTIONARY_FOR_TESTING_ARGS,  # if excluded, tests won't run for this algorithm
#   }
#
algo_dict = {
  'main': {
    'algorithm': {
      'sources': [ 'Algorithm.cc', 'AlgorithmFactory.cc', 'AlgorithmSequence.cc' ],
      'headers': [ 'Algorithm.h', 'AlgorithmBoilerplate.h', 'TypeDefs.h', 'AlgorithmSequence.h' ],
    },
    'validator': {
      'sources': [ 'Validator.cc' ],
      'headers': [ 'Validator.h' ],
    },
  },
  'example::ExampleAlgorithm': {
    'algorithm': {
      'sources': [ 'example/ExampleAlgorithm.cc' ],
      'headers': [ 'example/ExampleAlgorithm.h' ],
    },
    'configs':   [ 'example/ExampleAlgorithm.yaml' ],
    'test_args': { 'banks': ['REC::Particle'] },
  },
  'clas12::EventBuilderFilter': {
    'algorithm': {
      'sources': [ 'clas12/EventBuilderFilter.cc' ],
      'headers': [ 'clas12/EventBuilderFilter.h' ],
    },
    'configs':   [ 'clas12/EventBuilderFilter.yaml' ],
    'test_args': { 'banks': ['REC::Particle'] },
  },
  'clas12::ZVertexFilter': {
    'algorithm': {
      'sources': [ 'clas12/ZVertexFilter.cc' ],
      'headers': [ 'clas12/ZVertexFilter.h' ],
    },
    'configs':   [ 'clas12/ZVertexFilter.yaml' ],
    'test_args': { 'banks': ['REC::Particle'] },
  },
  'clas12::SectorFinder': {
    'algorithm': {
      'sources': [ 'clas12/SectorFinder.cc' ],
      'headers': [ 'clas12/SectorFinder.h' ],
    },
    'configs':   [ 'clas12/SectorFinder.yaml' ],
    'test_args': { 'banks': ['REC::Particle', 'REC::Calorimeter', 'REC::Track', 'REC::Scintillator'] },
  },
  'clas12::LorentzTransformer': {
    'algorithm': {
      'sources': [ 'clas12/LorentzTransformer.cc' ],
      'headers': [ 'clas12/LorentzTransformer.h' ],
      'needs_ROOT': true,
    },
    'configs':   [ 'clas12/LorentzTransformer.yaml' ],
    'test_args': { 'banks': ['REC::Particle'] },
  },
  'clas12::MomentumCorrection': {
    'algorithm': {
      'sources': [ 'clas12/MomentumCorrection.cc' ],
      'headers': [ 'clas12/MomentumCorrection.h' ],
    },
    'validator': {
      'sources': [ 'clas12/MomentumCorrectionValidator.cc' ],
      'headers': [ 'clas12/MomentumCorrectionValidator.h' ],
      'needs_ROOT': true,
    },
    'test_args': { 'banks': ['RUN::config', 'REC::Particle', 'REC::Calorimeter', 'REC::Track', 'REC::Scintillator'] },
  },
  'physics::InclusiveKinematics': {
    'algorithm': {
      'sources': [ 'physics/InclusiveKinematics.cc' ],
      'headers': [ 'physics/InclusiveKinematics.h' ],
      'needs_ROOT': true,
    },
    'validator': {
      'sources': [ 'physics/InclusiveKinematicsValidator.cc' ],
      'headers': [ 'physics/InclusiveKinematicsValidator.h' ],
      'needs_ROOT': true,
    },
    'configs': [ 'physics/InclusiveKinematics.yaml' ],
    'test_args': { 'banks': ['REC::Particle'] },
  },
}

# make lists of objects to build; inclusion depends on whether ROOT is needed or not, and if we have ROOT
algo_sources = []
algo_headers = []
algo_configs = []
vdor_sources = []
vdor_headers = []
foreach name, info : algo_dict
  # algorithms:
  info_algo = info.get('algorithm')
  algo_needs_ROOT = info_algo.get('needs_ROOT', false)
  if (algo_needs_ROOT and ROOT_dep.found()) or not algo_needs_ROOT
    algo_sources += info_algo.get('sources', [])
    algo_headers += info_algo.get('headers', [])
    algo_configs += info.get('configs', [])
  else
    warning('Excluding algorithm "' + name + '", which depends on ROOT')
  endif
  # validators:
  if info.has_key('validator')
    info_vdor = info.get('validator')
    vdor_needs_ROOT = info_vdor.get('needs_ROOT', false)
    if (vdor_needs_ROOT and ROOT_dep.found()) or not vdor_needs_ROOT
      vdor_sources += info_vdor.get('sources', [])
      vdor_headers += info_vdor.get('headers', [])
    endif
  endif
endforeach

# build and install algorithms
algo_lib = shared_library(
  'IguanaAlgorithms',
  algo_sources,
  include_directories: [ project_inc ] + ROOT_dep_inc_dirs,
  dependencies:        project_deps,
  link_with:           services_lib,
  link_args:           ROOT_dep_link_args,
  install:             true,
  build_rpath:         ROOT_dep_rpath,
)
install_headers(
  algo_headers,
  subdir:        meson.project_name() / 'algorithms',
  preserve_path: true,
)
project_libs += algo_lib

# build and install validators
vdor_lib = shared_library(
  'IguanaValidators',
  vdor_sources,
  include_directories: [ project_inc ] + ROOT_dep_inc_dirs,
  dependencies:        project_deps,
  link_with:           [ services_lib, algo_lib ],
  link_args:           ROOT_dep_link_args + ROOT_dep_link_args_for_validators,
  install:             true,
  build_rpath:         ROOT_dep_rpath,
)
install_headers(
  vdor_headers,
  subdir:        meson.project_name() / 'algorithms',
  preserve_path: true,
)

# install config files
foreach algo_config : algo_configs
  install_data(
    algo_config,
    install_dir:   project_etc / 'algorithms',
    preserve_path: true,
  )
endforeach
