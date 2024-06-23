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

# build and install the executable
example_bin = 'iguana_ex_cpp_00_run_functions'
executable(
  example_bin,
  example_bin + '.cc',
  dependencies:  [hipo_dep, iguana_dep],
  install:       true,
)
