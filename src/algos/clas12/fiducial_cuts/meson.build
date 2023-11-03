algo_headers = [
  'FiducialCuts.h',
]

algo_sources = [
  'FiducialCuts.cc',
]

algo_lib = shared_library(
  'algo_lib',
  algo_sources,
  include_directories : project_inc,
  install_rpath: project_lib_rpath,
  link_with : services_lib,
  install : true,
)

install_headers(algo_headers, subdir : meson.project_name())
