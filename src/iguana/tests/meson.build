test_exe_name = 'iguana-test'

test_exe = executable(
  test_exe_name,
  test_exe_name + '.cc',
  include_directories: project_inc,
  dependencies:        project_deps,
  link_with:           project_libs,
  install:             false,
  install_rpath:       ':'.join(project_exe_rpath),
)

# test algorithms
if fs.is_file(get_option('test_data_file'))
  foreach name, info : algo_dict
    if info.has_key('unit_test')
      needs_ROOT = info.get('needs_ROOT', false)
      if (needs_ROOT and ROOT_dep.found()) or not needs_ROOT
        bank_args = []
        foreach bank : info['unit_test'].get('banks', [])
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
  test('config-test_' + test_num, test_exe, args: [ '-c', 'config', '-n', test_num ])
endforeach
