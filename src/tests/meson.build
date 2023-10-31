exe_rpath = '$ORIGIN/../lib'
test_exe = executable(
  'test_exe',
  'main.cc',
  include_directories : project_inc,
  link_with : [algo_lib, iguana_lib],
  build_rpath: exe_rpath,
  install_rpath: exe_rpath,
  install : true,
)
