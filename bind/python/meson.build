project_pkg_vars += 'pythonpath=' + '${prefix}' / get_option('pythondir')

install_subdir(
  'pyiguana',
  install_dir: get_option('pythondir'),
)

python_examples = [
  'iguana-example-00-basic.py',
  'iguana-example-01-bank-rows.py',
]

foreach example : python_examples
  install_data(
    example,
    install_dir:  get_option('bindir'),
    install_mode: 'rwxr-xr-x',
  )
endforeach
