# install config files
example_config_files_prefix = project_etc / 'examples'
install_subdir('config', install_dir: example_config_files_prefix, strip_directory: true)

# example source information
example_sources = {
  'iguana-example-basic': {
    'sources': [ 'iguana-example-basic.cc' ],
  },
  'iguana-example-bank-rows': {
    'sources': [ 'iguana-example-bank-rows.cc' ],
  },
  'iguana-example-dataframes': {
    'sources': [ 'iguana-example-dataframes.cc' ],
    'build_this': ROOT_dep.found() and hipo_dep_dataframes_found,
  },
  'iguana-example-config-files': {
    'sources':   [ 'iguana-example-config-files.cc' ],
    'test_args': [ get_option('prefix') / example_config_files_prefix ],
  },
  'iguana-example-fortran': {
    'sources':     [ 'iguana-example-fortran.f' ],
    'build_this':  get_option('bind_fortran') and ROOT_dep.found(), # depends on physics::InclusiveKinematics, which depends on ROOT
    'link_with':   get_option('bind_fortran') ? [ algo_lib, algo_fortran_lib ] : [],
  },
}

# build executables and test them
foreach example, info : example_sources
  if(info.get('build_this', true))
    example_exe = executable(
      example,
      sources:             info['sources'],
      include_directories: [ project_inc ] + ROOT_dep_inc_dirs,
      dependencies:        project_deps,
      link_with:           info.get('link_with', project_libs),
      link_args:           ROOT_dep_link_args + ROOT_dep_link_args_for_validators,
      install:             true,
      build_rpath:         ROOT_dep_rpath,
    )
    if fs.is_file(get_option('test_data_file'))
      test(
        example,
        example_exe,
        suite: ['example'],
        args: info.get(
          'test_args',
          [ get_option('test_data_file'), '100' ] # don't run too many events, or meson test log will be huge
        ),
        env: project_test_env,
        timeout: 60,
      )
    endif
  endif
endforeach
