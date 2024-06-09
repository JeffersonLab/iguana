# start the algorithm dictionary list with the infrastructure sources
algo_dict = [
  {
    'name': 'main',
    'directory': '.',
    'has_config': false,
    'has_c_bindings': true,
    'has_validator': true,
    'algorithm_needs_ROOT': false,
    'validator_needs_ROOT': false,
    'add_algorithm_sources': [ 'AlgorithmFactory.cc', 'AlgorithmSequence.cc' ],
    'add_algorithm_headers': [ 'AlgorithmBoilerplate.h', 'TypeDefs.h', 'AlgorithmSequence.h' ],
  }
]

# list of dictionaries for info about each algorithm
#
#  example:
#  ========
#   {
#     'name':                str         # algorithm name (REQUIRED)
#     'directory':           str         # algorithm source code directory (default is based on 'name', replacing '::' with '/')
#     'has_config':          bool        # assumes Config.yaml exists (default=true)
#     'has_c_bindings':      bool        # assumes Bindings.cc exists (default=true)
#     'has_validator':       bool        # assumes Validator{cc,h} exist (default=true)
#     'algorithm_needs_ROOT': bool       # whether this algorithm needs ROOT or not (default=false)
#     'validator_needs_ROOT': bool       # whether this validator needs ROOT or not (default=true)
#     'add_algorithm_sources': list[str] # list of additional algorithm source files (default=[])
#     'add_algorithm_headers': list[str] # list of additional algorithm header files (default=[])
#     'add_validator_sources': list[str] # list of additional validator source files (default=[])
#     'add_validator_headers': list[str] # list of additional validator header files (default=[])
#     'test_args': {    dict[str],       # if excluded, tests won't run for this algorithm
#       'banks':          list[str]      # list of banks that are needed to test this algorithm
#       'prerequisites':  list[str]      # list of algorithms that that are required to `Run` before this one
#     },
#   }
#
algo_dict += [
  {
    'name': 'example::ExampleAlgorithm',
    'has_c_bindings': false,
    'has_validator': false,
    'test_args': { 'banks': ['REC::Particle'] },
  },
  {
    'name': 'clas12::EventBuilderFilter',
    'has_validator': false,
    'test_args': { 'banks': ['REC::Particle'] },
  },
  {
    'name': 'clas12::ZVertexFilter',
    'has_validator': false,
    'test_args': { 'banks': ['REC::Particle', 'RUN::config'] },
  },
  {
    'name': 'clas12::SectorFinder',
    'has_c_bindings': false,
    'has_validator': false,
    'test_args': { 'banks': ['REC::Particle', 'REC::Calorimeter', 'REC::Track', 'REC::Scintillator'] },
  },
  {
    'name': 'clas12::LorentzTransformer',
    'algorithm_needs_ROOT': true,
    'has_c_bindings': false,
    'has_validator': false,
    'test_args': { 'banks': ['REC::Particle'] },
  },
  {
    'name': 'clas12::FTEnergyCorrection',
    'has_config': false,
    'has_c_bindings': false,
    'has_validator': false,
    'test_args': { 'banks': ['RECFT::Particle'] },
  },
  {
    'name': 'clas12::MomentumCorrection',
    'has_config': false,
    'test_args': {
      'banks': ['RUN::config', 'REC::Particle', 'REC::Calorimeter', 'REC::Track', 'REC::Scintillator'],
      'prerequisites': ['clas12::SectorFinder'],
    },
  },
  {
    'name': 'physics::InclusiveKinematics',
    'algorithm_needs_ROOT': true,
    'test_args': { 'banks': ['REC::Particle'] },
  },
  {
    'name': 'clas12::PhotonGBTFilter',
    'has_c_bindings': false,
    'algorithm_needs_ROOT': true,
    'has_validator': true,
    'test_args': { 'banks': ['REC::Particle', 'REC::Calorimeter', 'RUN::config'] },
  }
]

# make lists of objects to build; inclusion depends on whether ROOT is needed or not, and if we have ROOT
algo_sources = []
algo_headers = []
algo_configs = []
vdor_sources = []
vdor_headers = []

foreach algo : algo_dict

  algo_name           = algo.get('name')
  algo_dir            = algo.get('directory', '/'.join(algo_name.split('::')))
  algo_has_config     = algo.get('has_config', true)
  algo_has_c_bindings = algo.get('has_c_bindings', true)
  algo_has_validator  = algo.get('has_validator', true)
  algo_needs_ROOT     = algo.get('algorithm_needs_ROOT', false)
  vdor_needs_ROOT     = algo.get('validator_needs_ROOT', true)

  # algorithms:
  if (algo_needs_ROOT and ROOT_dep.found()) or not algo_needs_ROOT

    # sources and headers
    algo_sources += algo_dir / 'Algorithm.cc'
    algo_headers += algo_dir / 'Algorithm.h'
    foreach src_file : algo.get('add_algorithm_sources', [])
      algo_sources += algo_dir / src_file
    endforeach
    foreach src_file : algo.get('add_algorithm_headers', [])
      algo_headers += algo_dir / src_file
    endforeach

    # C/Fortran bindings
    if get_option('bind_fortran')
      if algo_has_c_bindings
        algo_sources += algo_dir / 'Bindings.cc'
      else
        warning('Algorithm "' + algo_name + '" has no Fortran bindings')
      endif
    endif

    # config files
    if algo_has_config
      algo_configs += algo_dir / 'Config.yaml'
    endif

  else
    warning('Excluding algorithm "' + algo_name + '", which depends on ROOT')
  endif

  # validators:
  if algo_has_validator
    if (vdor_needs_ROOT and ROOT_dep.found()) or not vdor_needs_ROOT
      vdor_sources += algo_dir / 'Validator.cc'
      vdor_headers += algo_dir / 'Validator.h'
      foreach src_file : algo.get('add_validator_sources', [])
        vdor_sources += algo_dir / src_file
      endforeach
      foreach src_file : algo.get('add_validator_headers', [])
        vdor_headers += algo_dir / src_file
      endforeach
    endif
  else
    warning('Algorithm "' + algo_name + '" has no Validator; use at your own risk!')
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
