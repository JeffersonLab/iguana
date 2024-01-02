project(
  'iguana-build-example',
  'cpp',
  version: '1.0.0',
  default_options: [ 'cpp_std=c++17' ],
)

# find dependencies
# - it's good practice to specifiy minimum version requirements; here we just assume that `iguana` already did this
bin_deps = [
  dependency('hipo4'),
  dependency('iguana'),
]

# set rpath
# - this is so that the executable knows where the dependency libraries are
# - alternatively, set $LD_LIBRARY_PATH before running your executables
bin_rpaths = []
foreach dep : bin_deps
  bin_rpaths += dep.get_variable('libdir')
endforeach

# build and install the executable
example_bin = 'iguana-example-00-basic'
executable(
  example_bin,
  example_bin + '.cc',
  dependencies:  bin_deps,
  install:       true,
  install_rpath: ':'.join(bin_rpaths),
)
