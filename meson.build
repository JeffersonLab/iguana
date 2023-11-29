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
'$ORIGIN/../' + project_lib_install_dir,
  get_option('hipo') + '/lib',
]
cmake_prefix_paths = [
  get_option('hipo'),
]
if get_option('fmt') != ''
  dep_lib_rpaths += get_option('fmt') + '/lib'
  cmake_prefix_paths += get_option('fmt')
endif

project_lib_rpath = '$ORIGIN'
project_bin_rpath = ':'.join(dep_lib_rpaths)
cmake_prefix_path = '-DCMAKE_PREFIX_PATH=' + ';'.join(cmake_prefix_paths)

hipo_dep = dependency('hipo4', method: 'cmake', cmake_args: cmake_prefix_path)
fmt_dep  = dependency('fmt', method: 'cmake', cmake_args: cmake_prefix_path)

subdir('src/services')
subdir('src/algorithms')
subdir('src/iguana')
subdir('src/tests')
