example_sources = [
  'iguana-example-00-basic.cc',
  'iguana-example-01-bank-rows.cc',
  'iguana-example-02-YAMLReader.cc',
  'iguana-example-03-zvertex-filter.cc',
]

# build executables
foreach src : example_sources
  executable(
    src.split('.')[0],
    src,
    include_directories: [ project_inc, include_directories(root_dep.get_variable(cmake: 'PACKAGE_INCLUDE_DIRS')) ],
    dependencies:        project_deps,
    link_with:           project_libs,
    link_args: [
      '-L/home/dilks/builds/root/lib',
      '-lCore',
      '-lImt',
      '-lRIO',
      '-lNet',
      '-lHist',
      '-lGraf',
      '-lGraf3d',
      '-lGpad',
      '-lROOTVecOps',
      '-lTree',
      '-lTreePlayer',
      '-lRint',
      '-lPostscript',
      '-lMatrix',
      '-lPhysics',
      '-lMathCore',
      '-lThread',
      '-lMultiProc',
      '-lROOTDataFrame',
      # '-Wl,-rpath,/home/dilks/builds/root/lib',
      # '-pthread',
      # '-lm',
      # '-ldl',
      # '-rdynamic',
    ],
    install:             true,
    install_rpath:       ':'.join(project_exe_rpath),
  )
endforeach

# install config files
install_subdir(
  'config',
  install_dir:     project_etc / 'examples',
  strip_directory: true,
)
