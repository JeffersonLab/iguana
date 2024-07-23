if use_chameleon

  algo_bind_c_sources += [ 'Bindings.cc' ]
  algo_bind_c_headers = [ 'Bindings.h' ]

  # build C bindings
  if get_option('bind_fortran')
    algo_bind_c_lib = shared_library(
      'IguanaBindingsC',
      algo_bind_c_sources,
      include_directories: [ project_inc ] + ROOT_dep_inc_dirs,
      dependencies: project_deps,
      link_with: [ services_lib, algo_lib ],
      link_args: ROOT_dep_link_args + ROOT_dep_link_args_for_validators,
      install: true,
      build_rpath: ROOT_dep_rpath,
    )
    project_libs += algo_bind_c_lib
  endif
  install_headers(
    algo_bind_c_headers,
    subdir: meson.project_name() / 'bindings',
    preserve_path: true,
  )

endif
