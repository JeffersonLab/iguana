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
# - alternatively, set $LD_LIBRARY_PATH before running your executables ($DYLD_LIBRARY_PATH on macOS)
# - not needed for libraries which are static-linked
bin_rpaths = [
  hipo_dep.get_variable(pkgconfig: 'libdir')
]
if host_machine.system() != 'darwin'
  bin_rpaths += iguana_dep.get_variable(pkgconfig: 'libdir') # not needed on macOS
endif

# build and install the executable
example_bin = 'iguana-example-00-basic'
executable(
  example_bin,
  example_bin + '.cc',
  dependencies:  [hipo_dep, iguana_dep],
  install:       true,
  install_rpath: ':'.join(bin_rpaths),
)
