project(
  'rcdb',
  'cpp',
  meson_version: '>=1.2',
  version: 'iguana subproject'
)

# check if RCDB is installed
use_rcdb = false
rcdb_home = get_option('home')
rcdb_includedir = get_option('home') / 'cpp' / 'include'
if get_option('home') != ''
  fs = import('fs')
  if fs.is_dir(rcdb_includedir)
    use_rcdb = true
  else
    warning(f'RCDB include directory "@rcdb_includedir@" does not exist, so RCDB will not be used; are you sure your build option "rcdb:home" is correct?')
  endif
else
  warning('RCDB not found; set build option "rcdb:home" to the RCDB installation (e.g., "$RCDB_HOME") if you want to use it, but it is not strictly required.')
endif

# RCDB dependencies: either mysql or sqlite
# - looking for mysql here, since clas12root uses this
# - prioritizing mariadb, an open source fork of mysql
if use_rcdb
  rcdb_backend_dep = dependency('mariadb', required: false)
  if not rcdb_backend_dep.found()
    message('"mariadb" not found... looking for "mysql" instead...')
    rcdb_backend_dep = dependency('mysql', required: false)
    if not rcdb_backend_dep.found()
      warning('neither "mariadb" nor "mysql" were found, so RCDB will not be used in this build')
      use_rcdb = false
    endif
  endif
endif

rcdb_dep = not use_rcdb ? dependency('', required: false) : declare_dependency(
  version: get_option('home'), # RCDB lacks an easily accessible version number; instead just use the prefix
  variables: {
    'name': meson.project_name(),
    'includedir': rcdb_includedir,
  },
  include_directories: include_directories(
    rcdb_includedir,
    is_system: true, # suppress consumer warnings
  ),
  compile_args: ['-DRCDB_MYSQL' ],
  dependencies: [ rcdb_backend_dep ],
)
