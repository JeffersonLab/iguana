project(
  'iguana',
  'cpp',
  version: '0.0.0',
  license: 'LGPLv3'
)

project_inc = include_directories('src')

project_lib_install_dir = 'lib' # to keep it the same for different Linux distributions
project_exe_install_dir = 'bin'

project_lib_rpath = '$ORIGIN'
project_exe_rpath = ':'.join([
  '$ORIGIN/../' + project_lib_install_dir,
  get_option('hipo') + '/lib'
])

fmt_dep  = dependency('fmt')
hipo_dep = dependency('hipo4', method: 'cmake', cmake_args: '-DCMAKE_PREFIX_PATH=' + get_option('hipo'))

subdir('src/services')
subdir('src/algorithms/clas12/fiducial_cuts')
subdir('src/iguana')
subdir('src/tests')
