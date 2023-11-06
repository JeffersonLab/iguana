iguana_headers = [
  'Arbiter.h',
]

iguana_sources = [
  'Arbiter.cc',
]

iguana_lib = shared_library(
  'Iguana',
  iguana_sources,
  include_directories: project_inc,
  link_with:           [ algo_lib, services_lib ],
  install_rpath:       project_lib_rpath,
  install:             true,
)

install_headers(iguana_headers, subdir : meson.project_name())
