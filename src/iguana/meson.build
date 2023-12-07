iguana_headers = [
  'AlgorithmSequence.h',
]

iguana_sources = [
  'AlgorithmSequence.cc',
]

iguana_lib = shared_library(
  'Iguana',
  iguana_sources,
  include_directories: project_inc,
  dependencies:        [ fmt_dep, hipo_dep ],
  link_with:           [ algo_lib, services_lib ],
  install:             true,
  install_rpath:       project_lib_rpath,
)

install_headers(iguana_headers, subdir : meson.project_name())
