project_pkg_vars += 'pythonpath=' + '${prefix}' / pythondir

install_subdir('pyiguana', install_dir: pythondir)

if (get_option('install_examples'))
  python_examples = [
    'iguana_ex_python_00_run_functions.py',
    'iguana_ex_python_01_action_functions.py',
    'iguana_ex_python_hipopy.py',
  ]
  foreach example : python_examples
    install_data(example, install_dir: get_option('bindir'), install_mode: 'rwxr-xr-x')
  endforeach
endif
