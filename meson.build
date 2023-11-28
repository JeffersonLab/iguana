project(
  'iguana',
  'cpp',
  version: '0.0.0',
  license: 'LGPLv3',
  default_options: [ 'cpp_std=c++17' ],
)

project_inc = include_directories('src')

project_lib_install_dir = 'lib' # to keep it the same for different Linux distributions
project_bin_install_dir = 'bin'

dep_lib_rpaths = [
  get_option('hipo') + '/lib'
]
if get_option('fmt') != ''
  dep_lib_rpaths += get_option('fmt') + '/lib'
endif
project_lib_rpath = '$ORIGIN'
project_bin_rpath = ':'.join(['$ORIGIN/../' + project_lib_install_dir] + dep_lib_rpaths)

fmt_dep  = dependency(
  'fmt',
  cmake_args: get_option('fmt')!='' ? '-DCMAKE_PREFIX_PATH='+get_option('fmt') : '',
)
hipo_dep = dependency(
  'hipo4',
  cmake_args: '-DCMAKE_PREFIX_PATH='+get_option('hipo'),
)

subdir('src/services')
subdir('src/algorithms')
subdir('src/iguana')
subdir('src/tests')
