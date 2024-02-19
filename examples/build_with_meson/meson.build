project(
  'iguana-build-example',
  'cpp',
  version: '1.0.0',
  default_options: [ 'cpp_std=c++17' ],
)

# find dependencies
# - it's good practice to specifiy minimum version requirements; here we just assume that `iguana` already did this
# - if iguana was built with `static_hipo=True`, the `hipo4` dependency is not needed here
iguana_dep = dependency('iguana')

# set rpath
# - this is so that the executable knows where the dependency libraries are
# - alternatively, set $LD_LIBRARY_PATH before running your executables ($DYLD_LIBRARY_PATH on macOS)
bin_rpaths = [
]
if host_machine.system() != 'darwin'
  # FIXME(darwin): not sure how to set multiple rpaths on darwin executables, aside
  # from running `install_name_tool -add_rpath` post-installation; luckily,
  # darwin-built examples only need `hipo_dep` libraries in the rpath and they
  # don't need the `iguana` library path explictly included in the rpath, so
  # this `if` block just keeps `bin_rpaths` to only one element. If we need
  # more rpath elements someday, either we need to use `install_name_tool` or,
  # preferably, resolution of https://github.com/mesonbuild/meson/issues/5760
  bin_rpaths += iguana_dep.get_variable(pkgconfig: 'libdir')
endif

# build and install the executable
example_bin = 'iguana-example-00-basic'
executable(
  example_bin,
  example_bin + '.cc',
  dependencies:  [iguana_dep],
  install:       true,
  install_rpath: ':'.join(bin_rpaths),
)
