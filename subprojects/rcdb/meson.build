project(
  'rcdb',
  'cpp',
  meson_version: '>=1.2',
  version: 'iguana subproject'
)

use_rcdb = false
rcdb_home = get_option('home')
includedir = get_option('home') / 'cpp' / 'include'

if get_option('home') != ''
  fs = import('fs')
  if fs.is_dir(includedir)
    use_rcdb = true
  else
    warning(f'RCDB include directory "@includedir@" does not exist; RCDB will not be used')
  endif
else
  warning('RCDB not found; either use build option "rcdb:home" or set environment variable "$RCDB_HOME"')
endif

sqlite_dep = dependency('sqlite3', required: use_rcdb)
#
#
### clas12root consumes rcdb using mysql, NOT sqlite
#
#

rcdb_dep = not use_rcdb ? dependency('', required: false) : declare_dependency(
  # RCDB lacks a public version number, use its prefix instead to at least
  # inform the user which RCDB installation is used; alternatively, we could
  # use `module info-loaded rcdb` on ifarm, but it's better to use a
  # platform-independent approach
  version: get_option('home'),
  variables: {
    'name': meson.project_name(),
    'includedir': includedir,
  },
  include_directories: include_directories(
    includedir,
    is_system: true, # suppress consumer warnings
  ),
  dependencies: [ sqlite_dep ],
)
