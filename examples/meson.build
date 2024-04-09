# install config files
example_config_files_prefix = project_etc / 'examples'
install_subdir('config', install_dir: example_config_files_prefix, strip_directory: true)

# example source information
example_sources = {
  'iguana-example-00-basic': {
    'sources': [ 'iguana-example-00-basic.cc' ],
  },
  'iguana-example-01-bank-rows': {
    'sources': [ 'iguana-example-01-bank-rows.cc' ],
  },
  'iguana-example-02-config-files': {
    'sources':   [ 'iguana-example-02-config-files.cc' ],
    'test_args': [ get_option('prefix') / example_config_files_prefix ],
  },
}

# build executables and test them
foreach example, info : example_sources
  example_exe = executable(
    example,
    sources:             info['sources'],
    include_directories: [ project_inc ] + ROOT_dep_inc_dirs,
    dependencies:        project_deps,
    link_with:           project_libs,
    link_args:           ROOT_dep_link_args,
    install:             true,
    install_rpath:       ':'.join(project_exe_rpath),
    build_rpath:         ROOT_dep_rpath,
  )
  if fs.is_file(get_option('test_data_file'))
    test(
      example,
      example_exe,
      args: info.get(
        'test_args',
        [ get_option('test_data_file'), '100' ] # don't run too many events, or meson test log will be huge
      ),
      env: project_test_env,
    )
  endif
endforeach
