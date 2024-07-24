project(
  'rcdb',
  'cpp',
  meson_version: '>=1.2',
)

use_rcdb = false
rcdb_home = get_option('home')
rcdb_headers = get_option('home') / 'cpp' / 'include'

if get_option('home') != ''
  fs = import('fs')
  if fs.is_dir(rcdb_headers)
    use_rcdb = true
  else
    warning(f'RCDB include directory "@rcdb_headers@" does not exist; RCDB will not be used')
  endif
else
  warning('RCDB not found; either use build option "rcdb:home" or set environment variable "$RCDB_HOME"')
endif

rcdb_dep = not use_rcdb ? dependency('', required: false) : declare_dependency(
  version: get_option('home'), # could use `module info-loaded rcdb` on ifarm, but `get_option('home')` is platform independent
  include_directories: include_directories(
    rcdb_headers,
    is_system: true, # suppress consumer warnings
  ),
)
