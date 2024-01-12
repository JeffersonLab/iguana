example_sources = [
  'iguana-example-00-basic.cc',
  'iguana-example-01-bank-rows.cc',
]

# add dependencies' shared libraries to rpath
example_rpaths = [
  hipo_dep.get_variable(pkgconfig: 'libdir'),
]
if host_machine.system() != 'darwin'
  # FIXME(darwin): not sure how to set multiple rpaths on darwin executables, aside
  # from running `install_name_tool -add_rpath` post-installation; luckily,
  # darwin-built examples only need `hipo_dep` libraries in the rpath and they
  # don't need the `iguana` library path explictly included in the rpath, so
  # this `if` block just keeps `example_rpaths` to only one element. If we need
  # more rpath elements someday, either we need to use `install_name_tool` or,
  # preferably, resolution of https://github.com/mesonbuild/meson/issues/5760
  example_rpaths += '$ORIGIN' / '..' / get_option('libdir')
endif

# build executables
foreach src : example_sources
  executable(
    src.split('.')[0],
    src,
    include_directories: project_inc,
    dependencies:        project_deps,
    link_with:           project_libs,
    install:             true,
    install_rpath:       ':'.join(example_rpaths),
  )
endforeach
