test_bin = executable(
  'run',
  'main.cc',
  include_directories: project_inc,
  dependencies:        [ fmt_dep, hipo_dep ],
  link_with:           iguana_lib,
  install:             true,
  install_dir:         project_bin_install_dir,
  install_rpath:       project_bin_rpath,
)
