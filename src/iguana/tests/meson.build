test_exe_name = 'iguana-test'
test_exe_inc = include_directories('include')

test_exe = executable(
  test_exe_name,
  test_exe_name + '.cc',
  include_directories: [ test_exe_inc, project_inc ],
  dependencies:        project_deps,
  link_with:           project_libs,
  install:             false,
  install_rpath:       ':'.join(project_exe_rpath),
)

# test algorithms
if fs.is_file(get_option('test_data_file'))
  foreach name, info : algo_dict
    if info.has_key('test_args')
      algo_needs_ROOT = info.get('algorithm').get('algo_needs_ROOT', false)
      if (algo_needs_ROOT and ROOT_dep.found()) or not algo_needs_ROOT
        bank_args = []
        foreach bank : info['test_args'].get('banks', [])
          bank_args += ['-b', bank]
        endforeach
        test(
          'algorithm-' + name.replace('::', '-'),
          test_exe,
          args: [
            '-c', 'algorithm',
            '-f', get_option('test_data_file'),
            '-n', get_option('test_num_events'),
            '-a', name,
            '-v',
            ] + bank_args,
        )
      endif
    endif
  endforeach
else
  stat_file = get_option('test_data_file')=='' ? 'provided' : 'found'
  message('Test sample file NOT ' + stat_file + '; you may run algorithm tests manually with `' + get_option('prefix') / get_option('bindir') / test_exe_name + '`')
endif

# test config files
foreach test_num : [ '1', '2', '3' ]
  config_file_name = 'test_' + test_num + '.yaml'
  config_file = fs.copyfile('config' / config_file_name, config_file_name)
  test('config-test_' + test_num, test_exe, args: [ '-c', 'config', '-t', test_num, '-v' ])
endforeach
