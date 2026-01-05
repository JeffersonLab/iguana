services_sources = [
  'Logger.cc',
  'Object.cc',
  'ConfigFileReader.cc',
  'YAMLReader.cc',
  'ConcurrentParam.cc',
  'GlobalParam.cc',
  'RCDBReader.cc',
  'Tools.cc',
  'DataFileReader.cc',
]
services_headers = [
  'Logger.h',
  'Object.h',
  'ConfigFileReader.h',
  'YAMLReader.h',
  'ConcurrentParam.h',
  'GlobalParam.h',
  'RCDBReader.h',
  'Tools.h',
  'DataFileReader.h',
]

if rcdb_dep.found()
  add_project_arguments('-D USE_RCDB', language: ['cpp'])
endif

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
