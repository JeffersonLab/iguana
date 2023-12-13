project(
  'iguana-build-example',
  'cpp',
  version: '1.0.0',
  default_options: [ 'cpp_std=c++17' ],
)

# find dependencies
# - it's good practice to specifiy minimum version requirements; here we just assume that `iguana` already did this
hipo_dep   = dependency('hipo4')
iguana_dep = dependency('iguana')

# set rpath
# - this is so that the executable knows where the dependency libraries are
# - alternatively, set $LD_LIBRARY_PATH before running your executables
bin_rpaths = []
foreach path : get_option('cmake_prefix_path')
  bin_rpaths += path / 'lib'
endforeach
foreach path : get_option('pkg_config_path')
  bin_rpaths += path / '..'
endforeach

# build the executable
example_bin = 'iguana-example-00-basic'
executable(
  example_bin,
  example_bin + '.cc',
  dependencies:  [ hipo_dep, iguana_dep ],
  install:       true,
  install_rpath: ':'.join(bin_rpaths),
)
