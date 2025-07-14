bankdef_json = files('iguana.json')

prog_bankgen_sources = files('bankgen.py')
prog_bankgen = find_program(prog_bankgen_sources)

# generate BankDefs.{h,cc} from the JSON data model file
bankdef_tgt = custom_target(
  'bankdefs',
  input: [
    bankdef_json,
    prog_bankgen_sources,
  ],
  output: [ 'BankDefs.h', 'BankDefs.cc' ],
  command: [ prog_bankgen, '@INPUT0@', '@OUTDIR@/BankDefs' ],
  install: true,
  install_dir: [ get_option('includedir') / meson.project_name() / 'bankdefs', false ],
)

# install the JSON data model file; iguana won't need it, but it can be useful for user reference
install_data(bankdef_json, install_dir: project_etcdir / 'bankdefs' / 'hipo4')
