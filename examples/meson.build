# install config files
example_config_files_prefix = project_etcdir / 'examples'
install_subdir('config', install_dir: example_config_files_prefix, strip_directory: true)

# example source information
example_sources = {
  'iguana_ex_cpp_00_run_functions': {'sources': [ 'iguana_ex_cpp_00_run_functions.cc' ]},
  'iguana_ex_cpp_00_run_functions_with_banks': {'sources': [ 'iguana_ex_cpp_00_run_functions_with_banks.cc' ]},
  'iguana_ex_cpp_01_action_functions': {'sources': [ 'iguana_ex_cpp_01_action_functions.cc' ]},
  'iguana_ex_cpp_dataframes': {
    'sources': [ 'iguana_ex_cpp_dataframes.cc' ],
    'build_this': ROOT_dep.found() and hipo_dep_dataframes_found,
  },
  'iguana_ex_cpp_config_files': {
    'sources': [ 'iguana_ex_cpp_config_files.cc' ],
    'test_args': [ get_option('prefix') / example_config_files_prefix ],
  },
  'iguana_ex_fortran_01_action_functions': {
    'sources': [ 'iguana_ex_fortran_01_action_functions.f' ],
    'build_this': get_option('bind_fortran') and ROOT_dep.found(), # depends on physics::InclusiveKinematics, which depends on ROOT
  },
}

# build executables and test them
foreach example, info : example_sources
  if (info.get('build_this', true))
    example_exe = executable(
      example,
      sources: info['sources'],
      include_directories: [ project_inc ],
      dependencies: project_deps,
      link_with: project_libs,
      install: true,
    )
    if fs.is_file(get_option('test_data_file'))
      test(
        example,
        example_exe,
        suite: [ 'example', 'single_threaded' ],
        args: info.get(
          'test_args',
          [ get_option('test_data_file'), '100' ], # don't run too many events, or meson test log will be huge
        ),
        env: project_test_env,
        timeout: 240,
      )
    endif
  endif
endforeach
