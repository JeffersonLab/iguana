project(
  'rcdb',
  'cpp',
  meson_version: '>=1.2',
  version: 'iguana subproject',
)
require_message = 'RCDB is not required for Iguana, and you may proceed with building Iguana, but some algorithms may be easier to use with RCDB support.'

# check if RCDB is installed
rcdb_home = get_option('home')
# try `$RCDB_HOME`, if not set from build option
if rcdb_home == ''
  message('build option "rcdb:home" not set; trying environment variable "$RCDB_HOME"')
  rcdb_home = run_command('.' / 'get_rcdb_home.sh', check: true).stdout().strip()
endif
# error, if not found
if rcdb_home == ''
  error(f'RCDB not found; set build option "rcdb:home" to the RCDB installation (e.g., to "$RCDB_HOME"); @require_message@')
endif
# check for header dir
rcdb_includedir = rcdb_home / 'cpp' / 'include'
if not import('fs').is_dir(rcdb_includedir)
  error(f'RCDB include directory "@rcdb_includedir@" does not exist, so RCDB will not be used; are you sure your build option "rcdb:home" or environment variable "$RCDB_HOME" is correct?; @require_message@')
endif
# rcdb found
message(f'using RCDB from @rcdb_home@')

# RCDB dependencies: a database management system (DBMS), mariadb and/or sqlite
dbms_args = []
dbms_deps = []
dbms_dict = {
  'mariadb':  {
    'use': get_option('use_mariadb'),
    'pkgs': [ 'mariadb', 'libmariadb', 'mysql', 'mysqlclient' ], # prioritizing mariadb, an open source fork of mysql
  },
  'sqlite': {
    'use': get_option('use_sqlite'),
    'pkgs': [ 'sqlite3' ],
  },
}
foreach dbms, info : dbms_dict
  if info['use']
    foreach pkg : info['pkgs']
      dep = dependency(pkg, required: false)
      if dep.found()
        dbms_deps += dep
        dbms_args += { 'mariadb': '-DRCDB_MYSQL', 'sqlite': '-DRCDB_SQLITE' }[dbms]
        message(f'using @pkg@ for RCDB DBMS')
        break
      endif
    endforeach
  endif
endforeach
if dbms_deps.length() == 0
  error('none of the DBMSs (' + ', '.join(dbms_dict.keys()) + ') were found, so RCDB will not be used in this build; ' + require_message)
endif

rcdb_dep = declare_dependency(
  version: rcdb_home, # RCDB lacks an easily accessible version number; instead just use the prefix
  variables: {
    'name': meson.project_name(),
    'includedir': rcdb_includedir,
  },
  include_directories: include_directories(
    rcdb_includedir,
    is_system: true, # suppress consumer warnings
  ),
  compile_args: dbms_args,
  dependencies: dbms_deps,
)
