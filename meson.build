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

dep_lib_rpaths = [ '$ORIGIN/../' + project_lib_install_dir ]
dep_lib_rpaths += get_option('cmake_prefix_path') # TODO: this is for hipo4; consider instead linking against hipo4 static lib

project_lib_rpath = '$ORIGIN'
project_bin_rpath = ':'.join(dep_lib_rpaths)

hipo_dep = dependency('hipo4')
fmt_dep  = dependency('fmt')

subdir('src/services')
subdir('src/algorithms')
subdir('src/iguana')
subdir('src/tests')
