project(
  'rcdb',
  'cpp',
  meson_version: '>=1.2',
)

rcdb_home = ''
foreach dir : [
  get_option('home'),
  run_command('getenv_rcdb_home.sh', check: false).stdout().strip(),
]
  if dir != ''
    rcdb_home = dir
    message('RCDB found at ' + rcdb_home)
    break
  endif
endforeach

rcdb_dep = rcdb_home == '' ? dependency('', required: false) : declare_dependency(
  include_directories: include_directories(rcdb_home),
)
