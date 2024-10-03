project(
  'rcdb',
  'cpp',
  meson_version: '>=1.2',
  version: 'iguana subproject',
)
require_message = 'RCDB is not required for Iguana, and you may proceed with building Iguana, but some algorithms may be easier to use with RCDB support.'

# check if RCDB is installed
rcdb_home = get_option('home')
rcdb_includedir = get_option('home') / 'cpp' / 'include'
if get_option('home') != ''
  fs = import('fs')
  if not fs.is_dir(rcdb_includedir)
    error(f'RCDB include directory "@rcdb_includedir@" does not exist, so RCDB will not be used; are you sure your build option "rcdb:home" is correct?; @require_message@')
  endif
else
  error(f'RCDB not found; set build option "rcdb:home" to the RCDB installation (e.g., to "$RCDB_HOME"); @require_message@')
endif

# RCDB dependencies: either mysql or sqlite
# - looking for mysql here, since clas12root uses this
# - prioritizing mariadb, an open source fork of mysql
backend_list = ['mariadb', 'mysql', 'mysqlclient']
foreach backend_name : backend_list
  rcdb_backend_dep = dependency(backend_name, required: false)
  if rcdb_backend_dep.found()
    message(f'using "@backend_name@" for the RCDB backend')
    break
  endif
endforeach
if not rcdb_backend_dep.found()
  error('none of the backends (' + ', '.join(backend_list) + ') were found, so RCDB will not be used in this build; ' + require_message)
endif

rcdb_dep = declare_dependency(
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
