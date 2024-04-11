executable(
  'iguana-example-fortran',
  sources: ['iguana-example-fortran.f03'],
  include_directories: [ project_inc ],
  dependencies:        project_deps,
  link_with:           project_libs,
  install:             true,
  install_rpath:       ':'.join(project_exe_rpath),
)
