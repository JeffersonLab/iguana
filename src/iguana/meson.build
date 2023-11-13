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
  install:             true,
  install_dir:         project_lib_install_dir,
  install_rpath:       project_lib_rpath,
)

install_headers(iguana_headers, subdir : meson.project_name())
