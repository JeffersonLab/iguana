project(
  'iguana',
  'cpp',
  version : '0.0.0',
  license : 'LGPLv3'
)

project_inc = include_directories('src')
project_exe_rpath = '$ORIGIN/../lib'
project_lib_rpath = '$ORIGIN'

# dep_fmt    = dependency('fmt')
dep_spdlog = dependency('spdlog')

subdir('src/services')
subdir('src/algorithms/clas12/fiducial_cuts')
subdir('src/iguana')
subdir('src/tests')
