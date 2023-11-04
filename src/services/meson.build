services_headers = [
  'Logger.h',
  'Algorithm.h',
]

services_sources = [
  'Logger.cc',
  'Algorithm.cc',
]

services_lib = shared_library(
  'IguanaServices',
  services_sources,
  dependencies : [ dep_spdlog ],
  include_directories : project_inc,
  install_rpath: project_lib_rpath,
  install : true,
)

install_headers(services_headers, subdir : meson.project_name())
