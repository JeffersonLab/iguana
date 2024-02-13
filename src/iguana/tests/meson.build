test_exe_name = 'iguana-test'

test_exe = executable(
  test_exe_name,
  test_exe_name + '.cc',
  include_directories: project_inc,
  dependencies:        project_deps,
  link_with:           project_libs,
  install:             true,
  # install_rpath:     # don't bother with rpath, since `meson test` will run it in the build directory
)

if fs.is_file(get_option('test_data_file'))
  message('Test sample file provided; you may run tests with `meson test`')
  foreach algo, banks : algos_and_banks_for_unit_testing
    test(
      test_exe_name + ' ' + algo.replace('::', '__'),
      test_exe,
      args: [ get_option('test_data_file'), get_option('test_num_events'), algo ] + banks,
    )
  endforeach
else
  stat_file = get_option('test_data_file')=='' ? 'provided' : 'found'
  message('Test sample file NOT ' + stat_file + '; you may run tests manually with `' + test_exe_name + '`')
endif
