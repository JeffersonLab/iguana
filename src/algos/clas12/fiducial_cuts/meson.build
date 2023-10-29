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
  install : true,
)

install_headers(algo_headers, subdir : meson.project_name())
