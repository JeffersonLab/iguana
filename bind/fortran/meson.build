fortran_bindings_lib = shared_library(
  'IguanaFortranBindings',
  ['bind.f03'],
  include_directories: [ project_inc ],
  dependencies:        project_deps,
  link_with:           project_libs,
  link_args:           ROOT_dep_link_args,
  install:             true,
  install_rpath:       ':'.join(project_lib_rpath),
  build_rpath:         ROOT_dep_rpath,
)

executable(
  'iguana-example-fortran',
  sources: ['iguana-example-fortran.f03'],
  include_directories: [ project_inc ],
  dependencies:        project_deps,
  link_with:           project_libs + fortran_bindings_lib,
  install:             true,
  install_rpath:       ':'.join(project_exe_rpath),
)
