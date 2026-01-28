test_exe_name = 'iguana_test'
test_exe_inc = include_directories('include')
test_exe_links = project_libs + [ vdor_lib ]

test_exe = executable(
  test_exe_name,
  test_exe_name + '.cc',
  include_directories: [ test_exe_inc, project_inc ],
  dependencies: [ project_deps, thread_dep ],
  link_with: test_exe_links,
  install: true,
)

# test algorithms and validators
foreach algo : algo_dict
  if algo.has_key('test_args')

    algo_name = algo.get('name')
    algo_has_validator = algo.get('has_validator', true)
    algo_needs_ROOT = algo.get('algorithm_needs_ROOT', false)
    vdor_needs_ROOT = algo.get('validator_needs_ROOT', true)

    # test names cannot have colons
    test_name_algo = algo_name.replace('::', '-')

    # test and benchmark algorithm
    if (algo_needs_ROOT and ROOT_dep.found()) or not algo_needs_ROOT
      test_args = [ '-a', algo_name ]
      if fs.is_file(get_option('test_data_file'))
        test_args += [ '-f', get_option('test_data_file') ]
      endif
      foreach bank : algo['test_args'].get('banks', [])
        test_args += [ '-b', bank ]
      endforeach
      foreach prerequisite : algo['test_args'].get('prerequisites', [])
        test_args += [ '-p', prerequisite ]
      endforeach
      should_fail = algo['test_args'].get('should_fail', false)

      # single threaded tests
      test(
        '-'.join(['st', 'algorithm', test_name_algo]),
        test_exe,
        suite: [ 'algorithm', 'st' ],
        args: [ 'algorithm', '-n', get_option('test_num_events').to_string() ] + test_args,
        env: project_test_env,
        timeout: 0,
        should_fail: should_fail,
      )
      if not should_fail
        benchmark(
          '-'.join(['benchmark', 'st', test_name_algo]),
          test_exe,
          suite: [ 'algorithm', 'st' ],
          args: [ 'algorithm', '-n', '0' ] + test_args, # benchmark all the events
          env: project_test_env,
          timeout: 0,
        )
      endif

      # multithreaded tests
      if get_option('z_test_multithreading')
        multithreading_args = [
          'multithreading',
          '-j', get_option('test_num_threads').to_string(),
          '-m', 'memoize',
          '-V', # vary run number
        ]
        test(
          '-'.join(['mt', 'algorithm', test_name_algo]),
          test_exe,
          suite: [ 'algorithm', 'mt' ],
          args: multithreading_args + [ '-n', get_option('test_num_events').to_string() ] + test_args,
          is_parallel: false,
          env: project_test_env,
          timeout: 0,
          should_fail: should_fail,
        )
        if not should_fail
          benchmark(
            '-'.join(['benchmark', 'mt', test_name_algo]),
            test_exe,
            suite: [ 'algorithm', 'mt' ],
            args: multithreading_args + [ '-n', '0' ] + test_args, # benchmark all the events
            env: project_test_env,
            timeout: 0,
          )
        endif
      endif

    endif

    # test validator
    if algo_has_validator
      if (vdor_needs_ROOT and ROOT_dep.found()) or not vdor_needs_ROOT
        test_args = [
          'validator',
          '-n', get_option('test_validator_all_stats') ? '0' : get_option('test_num_events').to_string(),
          '-a', algo_name + 'Validator',
        ]
        if fs.is_file(get_option('test_data_file'))
          test_args += [ '-f', get_option('test_data_file') ]
        endif
        if get_option('test_output_dir') != ''
          vdor_output_dir = get_option('test_output_dir') / test_name_algo
          test_args += [ '-o', vdor_output_dir ]
        endif
        test(
          'validator-' + test_name_algo,
          test_exe,
          suite: [ 'validator' ],
          args: test_args,
          env: project_test_env,
          timeout: get_option('test_validator_all_stats') ? 0 : 120,
        )
      endif
    endif

  endif
endforeach

# test usage guide for each iguana_test command
foreach command : [ '', 'unknown', 'algorithm', 'validator', 'unit', 'config' ]
  test(
    'iguana_test-usage-guide' + (command == '' ? '' : '-' + command),
    test_exe,
    suite: [ 'misc' ],
    args: command == '' ? [] : [ command ],
    env: project_test_env,
    should_fail: true,
  )
endforeach

# test config files
foreach test_num : [ '1', '2', '3' ]
  config_file_name = 'test_' + test_num + '.yaml'
  config_file = fs.copyfile('config' / config_file_name, config_file_name)
  test(
    'config-test_' + test_num,
    test_exe,
    suite: [ 'misc' ],
    args: [ 'config', '-t', test_num, '-v' ],
    env: project_test_env,
  )
endforeach

# test Logger
test(
  'logger',
  test_exe,
  suite: [ 'misc' ],
  args: [ 'logger' ],
  env: project_test_env
)

# test banklist
if fs.is_file(get_option('test_data_file'))
  test(
    'banklist',
    test_exe,
    suite: [ 'misc' ],
    args: [ 'banklist', '-f', get_option('test_data_file') ],
    env: project_test_env
  )
endif
