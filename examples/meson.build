example_sources = [
  'iguana-example-00-basic.cc',
  'iguana-example-01-bank-rows.cc',
  'iguana-example-02-YAMLReader.cc',
  'iguana-example-03-zvertex-filter.cc',
]

# build executables
foreach src : example_sources
  executable(
    src.split('.')[0],
    src,
    include_directories: project_inc,
    dependencies:        project_deps,
    link_with:           project_libs,
    install:             true,
    install_rpath:       ':'.join(project_exe_rpath),
  )
endforeach

# install config files
install_subdir(
  'config',
  install_dir:     project_etc / 'examples',
  strip_directory: true,
)
