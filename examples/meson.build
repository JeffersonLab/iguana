# install config files
example_config_files_prefix = project_etc_dir / 'examples'
install_subdir('config' / 'examples', install_dir: example_config_files_prefix, strip_directory: true)

# compiled examples
example_sources = {
  # C++
  'iguana_ex_cpp_00_run_functions': {
    'sources': [ 'iguana_ex_cpp_00_run_functions.cc' ],
    'suite': 'cpp',
  },
  'iguana_ex_cpp_00_run_functions_with_banks': {
    'sources': [ 'iguana_ex_cpp_00_run_functions_with_banks.cc' ],
    'suite': 'cpp',
  },
  'iguana_ex_cpp_01_action_functions': {
    'sources': [ 'iguana_ex_cpp_01_action_functions.cc' ],
    'suite': 'cpp',
  },
  'iguana_ex_cpp_dataframes': {
    'sources': [ 'iguana_ex_cpp_dataframes.cc' ],
    'suite': 'cpp',
    'build_this': ROOT_dep.found() and hipo_dep_dataframes_found,
    'test_this': false, # FIXME: disabled for now, see https://github.com/JeffersonLab/iguana/issues/384
  },
  'iguana_ex_cpp_config_files': {
    'sources': [ 'iguana_ex_cpp_config_files.cc' ],
    'suite': 'cpp',
    'test_args': [ get_option('prefix') / example_config_files_prefix ],
  },
  # Fortran
  'iguana_ex_fortran_01_action_functions': {
    'sources': [ 'iguana_ex_fortran_01_action_functions.f' ],
    'suite': 'fortran',
    'build_this': get_option('bind_fortran') and ROOT_dep.found(), # depends on physics::InclusiveKinematics, which depends on ROOT
  },
}

# build executables and test them
foreach example, info : example_sources
  if info.get('build_this', true)
    example_exe = executable(
      example,
      sources: info['sources'],
      include_directories: [ project_inc ],
      dependencies: project_deps,
      link_with: project_libs,
      install: true,
    )
    if fs.is_file(get_option('test_data_file')) and info.get('test_this', true)
      test(
        example,
        example_exe,
        suite: [ 'example', info.get('suite') ],
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

# Python examples
if (get_option('bind_python'))
  python_examples = [
    'iguana_ex_python_00_run_functions.py',
    'iguana_ex_python_01_action_functions.py',
    'iguana_ex_python_hipopy.py',
  ]
  foreach example : python_examples
    install_data(
      example,
      install_dir: get_option('bindir'),
      install_mode: 'rwxr-xr-x'
    )
    if fs.is_file(get_option('test_data_file'))
      test(
        example,
        prog_python,
        suite: [ 'example', 'python' ],
        args: [ example, get_option('test_data_file'), '100' ], # don't run too many events, or meson test log will be huge
        workdir: meson.current_source_dir(),
        env: project_test_env,
        timeout: 240,
      )
    endif
  endforeach
endif
