services_headers = [
  'Algorithm.h',
]

services_sources = [
  'Algorithm.cc',
]

services_lib = shared_library(
  'services_lib',
  services_sources,
  include_directories : project_inc,
  install_rpath: project_lib_rpath,
  install : true,
)

install_headers(services_headers, subdir : meson.project_name())
