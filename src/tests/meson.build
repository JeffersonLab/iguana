foreach test_exe : [ 'run_banks', 'run_rows' ]
  executable(
    test_exe,
    test_exe + '.cc',
    include_directories: project_inc,
    dependencies:        [ fmt_dep, hipo_dep ],
    link_with:           [ iguana_lib, algo_lib, services_lib ],
    install:             true,
    install_dir:         project_bin_install_dir,
    install_rpath:       project_bin_rpath,
  )
endforeach
