prog_doxygen = find_program('doxygen')
prog_dot = find_program('dot', required: false)

doxyfile = configure_file(
  input: 'Doxyfile.in',
  output: 'Doxyfile',
  install: false,
  configuration: {
    'version': meson.project_version(),
    'have_dot': prog_dot.found() ? 'YES' : 'NO',
    'top_srcdir': meson.project_source_root(),
    'top_builddir': meson.project_build_root(),
  },
)

html_target = custom_target(
  'iguana-docs',
  input: doxyfile,
  output: 'html',
  command: [prog_doxygen, doxyfile],
  install: true,
  install_dir: join_paths(get_option('datadir'), 'doc', meson.project_name()),
)
