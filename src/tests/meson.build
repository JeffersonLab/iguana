test_exe = executable(
  'run',
  'main.cc',
  include_directories: project_inc,
  dependencies:        [ fmt_dep, hipo_dep ],
  link_with:           iguana_lib,
  install_rpath:       project_exe_rpath,
  install:             true,
)
