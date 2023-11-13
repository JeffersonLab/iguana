project(
  'iguana',
  'cpp',
  version: '0.0.0',
  license: 'LGPLv3'
)

project_inc = include_directories('src')

project_exe_rpath = ':'.join([
  '$ORIGIN/../lib',
  get_option('hipo') + '/lib'
])
project_lib_rpath = '$ORIGIN'

fmt_dep  = dependency('fmt')
hipo_dep = dependency('hipo4', method: 'cmake', cmake_args: '-DCMAKE_PREFIX_PATH=' + get_option('hipo'))

subdir('src/services')
subdir('src/algorithms/clas12/fiducial_cuts')
subdir('src/iguana')
subdir('src/tests')
