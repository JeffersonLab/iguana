project_pkg_vars += 'pythonpath=' + '${prefix}' / 'python'

install_subdir(
  'pyiguana',
  install_dir: 'python',
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
