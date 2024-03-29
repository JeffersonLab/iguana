test_exe_name = 'iguana-test'
test_exe_inc = include_directories('include')
test_exe_links = [ project_libs, vdor_lib ]

test_exe = executable(
  test_exe_name,
  test_exe_name + '.cc',
  include_directories: [ test_exe_inc, project_inc ],
  dependencies:        project_deps,
  link_with:           test_exe_links,
  link_args:           ROOT_dep_link_args,
  install:             true,
  install_rpath:       ':'.join(project_exe_rpath),
  build_rpath:         ROOT_dep_rpath,
)

# test algorithms and validators
if fs.is_file(get_option('test_data_file'))
  foreach name, info : algo_dict
    if info.has_key('test_args')

      # get list of banks
      bank_args = []
      foreach bank : info['test_args'].get('banks', [])
        bank_args += ['-b', bank]
      endforeach

      # test names cannot have colons
      test_name_algo = name.replace('::', '-')

      # test algorithm
      algo_needs_ROOT = info.get('algorithm').get('needs_ROOT', false)
      if (algo_needs_ROOT and ROOT_dep.found()) or not algo_needs_ROOT
        test_args = [
          'algorithm',
          '-f', get_option('test_data_file'),
          '-n', get_option('test_num_events'),
          '-a', name,
        ] + bank_args
        test(
          'algorithm-' + test_name_algo,
          test_exe,
          args: test_args,
          env: project_test_env,
        )
      endif

      # test validator
      if info.has_key('validator')
        vdor_needs_ROOT = info.get('validator').get('needs_ROOT', false)
        test_name_vdor = test_name_algo + 'Validator'
        if (vdor_needs_ROOT and ROOT_dep.found()) or not vdor_needs_ROOT
          test_args = [
            'validator',
            '-f', get_option('test_data_file'),
            '-n', get_option('test_num_events'),
            '-a', name + 'Validator',
          ]
          if get_option('test_output_dir') != ''
            vdor_output_dir = get_option('test_output_dir') / test_name_vdor
            run_command('mkdir', '-p', vdor_output_dir, check: true)
            test_args += [ '-o', vdor_output_dir ]
          endif
          test(
            'validator-' + test_name_vdor,
            test_exe,
            args: test_args,
            env: project_test_env,
          )
        endif
      endif

    endif
  endforeach
else
  stat_file = get_option('test_data_file')=='' ? 'provided' : 'found'
  message('Test sample file NOT ' + stat_file + '; you may run algorithm tests manually with `' + get_option('prefix') / get_option('bindir') / test_exe_name + '`')
endif

# test usage guide for each iguana-test command
foreach command : ['', 'unknown', 'algorithm', 'validator', 'unit', 'config']
  test(
    'iguana-test-usage-guide' + (command=='' ? '' : '-' + command),
    test_exe,
    args: command=='' ? [] : [command],
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
    args: [ 'config', '-t', test_num, '-v' ],
    env: project_test_env,
    )
endforeach

# test Logger
test(
  'logger',
  test_exe,
  args: [ 'logger' ],
  env: project_test_env,
)
