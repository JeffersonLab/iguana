iguana_headers = [
  # 'Algorithm.h',
  'Arbiter.h',
]

iguana_sources = [
  # 'Algorithm.cc',
  'Arbiter.cc',
]

iguana_lib = shared_library(
  'iguana_lib',
  iguana_sources,
  include_directories : project_inc,
  install : true,
)

install_headers(iguana_headers, subdir : meson.project_name())
