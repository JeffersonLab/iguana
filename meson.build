project(
  'iguana',
  'cpp',
  version: '0.0.0',
  license: 'LGPLv3',
  default_options: [ 'cpp_std=c++17' ],
  meson_version: '>=1.1'
)

project_inc = include_directories('src')

dep_lib_rpaths = [ ]
foreach path : get_option('cmake_prefix_path')
  dep_lib_rpaths += path + '/lib' # TODO: this adds hipo4 libs to the rpath; consider instead linking against hipo4 static lib
endforeach

project_lib_rpath = '$ORIGIN'
project_bin_rpath = ':'.join( dep_lib_rpaths + [ '$ORIGIN/../' + get_option('libdir') ] )

hipo_dep = dependency('hipo4', version: '>=1.3')
fmt_dep  = dependency('fmt', version: '>=9.1.0')

subdir('src/services')
subdir('src/algorithms')
subdir('examples')
