services_sources = [ 'Logger.cc', 'Object.cc', 'ConfigFileReader.cc', 'YAMLReader.cc', 'ConcurrentParam.cc', 'GlobalParam.cc' ]
services_headers = [ 'Logger.h', 'Object.h', 'ConfigFileReader.h', 'YAMLReader.h', 'ConcurrentParam.h', 'GlobalParam.h', 'TypeDefs.h' ]

services_lib = shared_library(
  'IguanaServices',
  services_sources,
  include_directories: project_inc,
  dependencies: project_deps,
  install: true,
)
project_libs += services_lib

install_headers(
  services_headers,
  subdir: meson.project_name() / 'services',
  preserve_path: true,
)
