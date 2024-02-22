test_exe_name = 'iguana-test'

test_exe = executable(
  test_exe_name,
  test_exe_name + '.cc',
  include_directories: project_inc,
  dependencies:        project_deps,
  link_with:           project_libs,
  install:             true,
  install_rpath:       ':'.join(project_exe_rpath),
)

if fs.is_file(get_option('test_data_file'))
  message('Test sample file provided; you may run tests with `meson test`')
  foreach name, info : algo_dict
    if info.has_key('unit_test')
      needs_ROOT = info.get('needs_ROOT', false)
      if (needs_ROOT and ROOT_dep.found()) or not needs_ROOT
        bank_args = []
        foreach bank : info['unit_test'].get('banks', [])
          bank_args += ['-b', bank]
        endforeach
        test(
          test_exe_name + '___' + name.replace('::', '-'),
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
  message('Test sample file NOT ' + stat_file + '; you may run tests manually with `' + get_option('prefix') / get_option('bindir') / test_exe_name + '`')
endif
