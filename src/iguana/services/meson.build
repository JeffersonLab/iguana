services_sources = [
  'Logger.cc',
  'Object.cc',
  'ConfigFileReader.cc',
  'YAMLReader.cc',
]

services_public_headers = [
  'Logger.h',
  'Object.h',
  'ConfigFileReader.h',
  'YAMLReader.h',
]

services_lib = shared_library(
  'IguanaServices',
  services_sources,
  include_directories: project_inc,
  dependencies:        project_deps,
  install:             true,
  install_rpath:       project_lib_rpath,
)
project_libs += services_lib

install_headers(
  services_public_headers,
  subdir:        meson.project_name() / 'services',
  preserve_path: true,
)
