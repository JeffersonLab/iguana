example_sources = [
  'iguana-example-00-basic.cc',
  'iguana-example-01-bank-rows.cc',
]

# add iguana libraries to rpath
example_rpaths = []
example_rpaths += hipo_dep.get_variable(pkgconfig: 'libdir')
if host_machine.system() != 'darwin'
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
