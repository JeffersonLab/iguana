executable(
  'iguana-example-fortran-01-bank-rows',
  sources: ['iguana-example-fortran-01-bank-rows.F'],
  include_directories: [ project_inc ],
  dependencies:        project_deps,
  link_with:           project_libs,
  install:             true,
  install_rpath:       ':'.join(project_exe_rpath),
)
