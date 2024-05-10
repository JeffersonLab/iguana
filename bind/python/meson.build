project_pkg_vars += 'pythonpath=' + '${prefix}' / pythondir

install_subdir(
  'pyiguana',
  install_dir: pythondir,
)

if(get_option('install_examples'))
  python_examples = [
    'iguana-example-00-basic.py',
    'iguana-example-01-bank-rows.py',
    'iguana-example-02-hipopy.py',
  ]
  foreach example : python_examples
    install_data(
      example,
      install_dir:  get_option('bindir'),
      install_mode: 'rwxr-xr-x',
    )
  endforeach
endif
