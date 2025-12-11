# list of dictionaries for info about each algorithm
#
#  example:
#  ========
#   {
#     'name':                str         # algorithm name (REQUIRED)
#     'directory':           str         # algorithm source code directory (default is based on 'name', replacing '::' with '/')
#     'has_config':          bool        # assumes Config.yaml exists (default=true)
#     'has_validator':       bool        # assumes Validator{cc,h} exist (default=true)
#     'has_action_yaml':     bool        # assumes Action.yaml exists (default=true)
#     'algorithm_needs_ROOT': bool       # whether this algorithm (or its prerequisites) needs ROOT or not (default=false)
#     'validator_needs_ROOT': bool       # whether this validator needs ROOT or not (default=true)
#     'add_algorithm_sources': list[str] # list of additional algorithm source files (default=[])
#     'add_algorithm_headers': list[str] # list of additional algorithm header files (default=[])
#     'add_algorithm_data':    list[str] # list of additional files to install (default=[])
#     'add_validator_sources': list[str] # list of additional validator source files (default=[])
#     'add_validator_headers': list[str] # list of additional validator header files (default=[])
#     'test_args': {    dict[str],       # if excluded, tests won't run for this algorithm
#       'banks':          list[str]      # list of banks that are needed to test this algorithm; exclude banks produced by 'prerequisites' algorithms (default=[])
#       'prerequisites':  list[str]      # list of algorithms that that are required to `Run` before this one (default=[])
#       'should_fail':    bool           # if true, test should fail; use this for deprecated algorithms (default=false)
#     },
#   }
#
algo_dict = [
  {
    'name': 'example::ExampleAlgorithm',
    'has_validator': false,
    'has_action_yaml': false,
    'test_args': {'banks': [ 'REC::Particle' ]},
  },
  {
    'name': 'clas12::EventBuilderFilter',
    'has_validator': false,
    'test_args': {'banks': [ 'REC::Particle' ]},
  },
  {
    'name': 'clas12::ZVertexFilter',
    'test_args': {'banks': [ 'REC::Particle', 'RUN::config' ]},
  },
  {
    'name': 'clas12::rga::FiducialFilterPass1',
    'test_args': {
      'banks': [ 'REC::Particle', 'RUN::config', 'REC::Traj', 'REC::Calorimeter' ],
      'prerequisites': [ 'clas12::CalorimeterLinker', 'clas12::TrajLinker' ],
    },
  },
  { 'name': 'clas12::FiducialFilter', 'has_config': false, 'has_validator': false, 'test_args': { 'should_fail': true } }, # deprecated
  {
    'name': 'clas12::SectorFinder',
    'test_args': {
      'banks': [ 'REC::Particle', 'REC::Calorimeter', 'REC::Track', 'REC::Scintillator' ],
    },
  },
  {
    'name': 'clas12::CalorimeterLinker',
    'has_config': false,
    'has_validator': false,
    'has_action_yaml': false,
    'test_args': {
      'banks': [ 'REC::Particle', 'REC::Calorimeter' ],
    },
  },
  {
    'name': 'clas12::TrajLinker',
    'has_config': false,
    'has_validator': false,
    'has_action_yaml': false,
    'test_args': {
      'banks': [ 'REC::Particle', 'REC::Traj' ],
    },
  },
  {
    'name': 'clas12::rga::FTEnergyCorrection',
    'has_config': false,
    'has_validator': false,
    'test_args': {'banks': [ 'RECFT::Particle' ]},
  },
  { 'name': 'clas12::FTEnergyCorrection', 'has_config': false, 'has_validator': false, 'test_args': { 'should_fail': true } }, # deprecated
  {
    'name': 'clas12::rga::MomentumCorrection',
    'has_config': false,
    'test_args': {
      'banks': [
        'RUN::config',
        'REC::Particle',
        'REC::Calorimeter',
        'REC::Track',
        'REC::Scintillator',
      ],
      'prerequisites': [ 'clas12::SectorFinder' ],
    },
  },
  { 'name': 'clas12::MomentumCorrection', 'has_config': false, 'has_validator': false, 'test_args': { 'should_fail': true } }, # deprecated
  {
    'name': 'physics::InclusiveKinematics',
    'algorithm_needs_ROOT': true,
    'test_args': {'banks': [ 'REC::Particle', 'RUN::config' ]},
  },
  {
    'name': 'physics::SingleHadronKinematics',
    'algorithm_needs_ROOT': true,
    'has_action_yaml': false, # FIXME: needs a vector action function
    'test_args': {
      'banks': [ 'REC::Particle', 'RUN::config' ],
      'prerequisites': [ 'physics::InclusiveKinematics' ],
    },
  },
  {
    'name': 'physics::DihadronKinematics',
    'algorithm_needs_ROOT': true,
    'has_action_yaml': false, # FIXME: needs a vector action function
    'test_args': {
      'banks': [ 'REC::Particle', 'RUN::config' ],
      'prerequisites': [ 'physics::InclusiveKinematics' ],
    },
  },
  {
    'name': 'physics::Depolarization',
    'algorithm_needs_ROOT': true, # since `physics::InclusiveKinematics` needs ROOT
    'has_config': false,
    'test_args': {
      'banks': [ ],
      'prerequisites': [ 'physics::InclusiveKinematics' ],
    },
  },
  {
    'name': 'clas12::PhotonGBTFilter',
    'algorithm_needs_ROOT': true,
    'has_action_yaml': false,
    'test_args': {'banks': [ 'REC::Particle', 'REC::Calorimeter', 'RUN::config' ]},
  },
  {
    'name': 'clas12::LeptonIDFilter',
    'algorithm_needs_ROOT': true,
    'has_validator': false,
    'has_action_yaml': false,
    'add_algorithm_data': [
      'weights/9_BDT_positrons_S19.weights.xml',
    ],
    'test_args': { 'banks': ['REC::Particle', 'REC::Calorimeter'] },
  },
  {
    'name': 'clas12::rga::FiducialFilterPass2',
    'has_validator': true,
    'has_action_yaml': false,
    'test_args': {
      # Only the required banks (sometiems REC::ForwardTagger does not exist)
      'banks': [ 'REC::Particle', 'RUN::config' ],
    },
  },
  { 'name': 'clas12::RGAFiducialFilter', 'has_config': false, 'has_validator': false, 'has_action_yaml': false, 'test_args': { 'should_fail': true } }, # deprecated
  {
    'name': 'clas12::MatchParticleProximity',
    'algorithm_needs_ROOT': true,
    'has_validator': false, # FIXME # need an MC file for non-empty validator plots
    'has_config': true,
    'has_action_yaml': false,
    'test_args': {
      'banks': [ 'REC::Particle', 'MC::Particle' ],
    },
>>>>>>> origin/main
  },
]

# make lists of objects to build; inclusion depends on whether ROOT is needed or not, and if we have ROOT
algo_sources = [
  'Algorithm.cc',
  'AlgorithmFactory.cc',
  'AlgorithmSequence.cc',
  bankdef_tgt[1], # BankDefs.cc
]
algo_headers = [
  'Algorithm.h',
  'AlgorithmBoilerplate.h',
  'TypeDefs.h',
  'AlgorithmSequence.h',
]
if ROOT_dep.found()
  algo_sources += [ 'physics/Tools.cc' ]
  algo_headers += [ 'physics/Tools.h' ]
endif
vdor_sources = [ 'Validator.cc' ]
vdor_headers = [ 'Validator.h' ]
algo_data    = []
algo_configs = []
algo_bind_c_sources = []

foreach algo : algo_dict

  algo_name = algo.get('name')
  algo_dir = algo.get('directory', '/'.join(algo_name.split('::')))
  algo_has_config = algo.get('has_config', true)
  algo_has_validator = algo.get('has_validator', true)
  algo_has_action_yaml = algo.get('has_action_yaml', true)
  algo_needs_ROOT = algo.get('algorithm_needs_ROOT', false)
  vdor_needs_ROOT = algo.get('validator_needs_ROOT', true)

  # algorithms:
  if (algo_needs_ROOT and ROOT_dep.found()) or not algo_needs_ROOT

    # sources, headers, and data
    algo_sources += algo_dir / 'Algorithm.cc'
    algo_headers += algo_dir / 'Algorithm.h'
    foreach src_file : algo.get('add_algorithm_sources', [])
      algo_sources += algo_dir / src_file
    endforeach
    foreach src_file : algo.get('add_algorithm_headers', [])
      algo_headers += algo_dir / src_file
    endforeach
    foreach src_file : algo.get('add_algorithm_data', [])
      algo_data += algo_dir / src_file
    endforeach

    # config files
    if algo_has_config
      algo_configs += algo_dir / 'Config.yaml'
    endif

    # run chameleon
    if use_chameleon and algo_has_action_yaml
      target_name = '_'.join([ 'chameleon', algo_name.split('::') ])
      tgt = custom_target(
        target_name,
        input: [ algo_dir / 'Action.yaml', chameleon_sources ],
        output: [ target_name + '_bind.cc' ],
        command: [
          prog_chameleon,
          '--input', '@INPUT0@',
          '--output', '@OUTDIR@',
          '--prefix', target_name,
          '--quiet',
          # '--debug',
        ],
      )
      algo_bind_c_sources += tgt[0]
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
  endif

endforeach

# build and install algorithms
algo_lib = shared_library(
  'IguanaAlgorithms',
  algo_sources,
  include_directories: [ project_inc ],
  dependencies: project_deps,
  link_with: [ services_lib ],
  install: true,
)
install_headers(algo_headers, subdir: meson.project_name() / 'algorithms', preserve_path: true)
foreach algo_datum : algo_data
  install_data(
    algo_data,
    install_dir:   project_data_dir / 'algorithms',
    preserve_path: true,
  )
endforeach
project_libs += algo_lib

# build and install validators
vdor_lib = shared_library(
  'IguanaValidators',
  vdor_sources,
  include_directories: [ project_inc ],
  dependencies: project_deps,
  link_with: [ services_lib, algo_lib ],
  install: true,
)
install_headers(vdor_headers, subdir: meson.project_name() / 'algorithms', preserve_path: true)

# install config files
foreach algo_config : algo_configs
  install_data(algo_config, install_dir: project_etc_dir / 'algorithms', preserve_path: true)
endforeach
