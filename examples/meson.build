# install config files
example_config_file = fs.copyfile(
  'config' / 'ex2.yaml',
  'ex2.yaml',
  install:     true,
  install_dir: project_etc / 'examples',
)

# example source information
example_sources = {
  'iguana-example-00-basic': {
    'sources': [ 'iguana-example-00-basic.cc' ],
  },
  'iguana-example-01-bank-rows': {
    'sources': [ 'iguana-example-01-bank-rows.cc' ],
  },
  'iguana-example-02-YAMLReader': {
    'sources':   [ 'iguana-example-02-YAMLReader.cc' ],
    'test_args': [ example_config_file.full_path() ],
  },
  'iguana-example-03-zvertex-filter': {
    'sources': [ 'iguana-example-03-zvertex-filter.cc' ],
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
  )
  if fs.is_file(get_option('test_data_file'))
    test(
      example,
      example_exe,
      args: info.get(
        'test_args',
        [ get_option('test_data_file'), get_option('test_num_events') ]
      ),
    )
  endif
endforeach
