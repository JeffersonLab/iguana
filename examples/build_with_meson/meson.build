project(
  'iguana-examples',
  'cpp',
  default_options: [ 'cpp_std=c++17' ],
)

fmt_dep    = dependency('fmt')
hipo_dep   = dependency('hipo4')
iguana_dep = dependency('iguana')

dep_lib_rpaths = [ ]
foreach path : get_option('cmake_prefix_path')
  dep_lib_rpaths += path / 'lib' # TODO: this adds hipo4 libs to the rpath; consider instead linking against hipo4 static lib
endforeach
project_bin_rpath = ':'.join( dep_lib_rpaths + [ '$ORIGIN/../' + get_option('libdir') ] )

foreach bin : [ '00-basic', '01-bank-rows' ]
  executable(
    'iguana-example-' + bin,
    bin + '.cc',
    dependencies:  [ fmt_dep, hipo_dep, iguana_dep ],
    install:       true,
    install_rpath: project_bin_rpath,
  )
endforeach
